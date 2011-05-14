/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 * Copyright (C) 2007 Jarosław Lewandowski <jotel@pi.net.pl>
 * Copyright (C) 2007 Andreas Schneider <mail@cynapses.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <webcam/V4L2WebcamDriver.h>

#include <pixertool/v4l2-pixertool.h>

#include <util/File.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <iostream>

#include <util/String.h>
#include <util/Logger.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

using namespace std;

V4L2WebcamDriver::V4L2WebcamDriver(WebcamDriver * driver, int flags)
	: IWebcamDriver(flags), _oldApiDriver(driver, flags) {
    _webcamDriver = driver;

    _fhandle = 0;
    buffers = NULL;
    n_buffers = 0;
    _oldApiDevice = false;
}

V4L2WebcamDriver::~V4L2WebcamDriver() {
    cleanup();
}

void V4L2WebcamDriver::cleanup() {
    if (_fhandle > 0) {
        close(_fhandle);
    }
    _fhandle = 0;
    _isOpen = false;
    _terminate = false;
    _fps = 15;

    _oldApiDriver.cleanup();
}


void V4L2WebcamDriver::terminate() {
    if (_oldApiDevice)
    {
        _oldApiDriver.terminate();
        return;
    }

    _terminate = true;
}

StringList V4L2WebcamDriver::getDeviceList() {
    StringList deviceList;
    DevNameArray devName = getDevices();

    DevNameArray::const_iterator i = devName.begin();

    while (i != devName.end()) {
        deviceList += ((*i).second);
        i++;
    }

    return deviceList;
}

string V4L2WebcamDriver::getDefaultDevice() {
    string defaultDeviceName;
    DevNameArray devName = getDevices();

    defaultDeviceName = devName["video0"];

    return defaultDeviceName;
}

WebcamErrorCode V4L2WebcamDriver::setDevice(const std::string & deviceName) {
    //TODO: test if a webcam is already open

    if (deviceName.empty()) {
        return WEBCAM_NOK;
    }

    std::string device = "/dev/" + deviceName.substr(deviceName.size() - 6, deviceName.size() - 1);

#if 0
    //Looking for the device path
    //FIXME: the current device naming scheme allows different devices to have
    //    the same name. This can produce some unwanted behavior (such as selecting
    //    the wrong device)
    string device;
    DevNameArray devName = getDevices();
    DevNameArray::const_iterator i = devName.begin();
    while (i != devName.end()) {
        if ((*i).second == deviceName) {
            device = "/dev/" + (*i).first;
            break;
        }
        i++;
    }
#endif

    try
    {
        struct stat st;
        const char * dev_name = device.c_str();

        LOG_DEBUG("Setting '"+device +"' device.");

        if (stat(dev_name, &st) < 0)
            throw runtime_error("can't stats device.");

        if (!S_ISCHR(st.st_mode))
            throw runtime_error("Isn't character device.");

        _fhandle = open(dev_name, O_RDWR | O_NONBLOCK);
        if (_fhandle <= 0)
            throw runtime_error("can't open '"+device+"'.");

        _isOpen = true;

        try {

            checkDevice();
            readCaps();

        } catch (runtime_error& ex) {
            LOG_DEBUG("Can't read capabilities probably not V4L2 device.");

            close(_fhandle);
            _isOpen = false;

            WebcamErrorCode ret = _oldApiDriver.setDevice(deviceName);

            if ( ret == WEBCAM_OK)
                _oldApiDevice = true;
            else
                LOG_DEBUG("Not supported device.");

            return ret;
        }

        _oldApiDevice = false;

        if (!(_vCaps.capabilities & V4L2_CAP_VIDEO_CAPTURE))
            throw runtime_error("Device is no video capture device\n.");

        if (!(_vCaps.capabilities & (V4L2_CAP_READWRITE | V4L2_CAP_STREAMING)))
            throw runtime_error("Device does not support streaming neither reading\n.");

        LOG_DEBUG("Device properly resolved.");

    } catch (runtime_error& ex) {
        LOG_ERROR(ex.what());

        return WEBCAM_NOK;
    }

    return WEBCAM_OK;
}

bool V4L2WebcamDriver::isOpen() const {
    if (_oldApiDevice)
        return _oldApiDriver.isOpen();

    return  _isOpen;
}

unsigned int V4L2WebcamDriver::reqDeviceBuffers(enum v4l2_memory mem, unsigned int num)
{
    struct v4l2_requestbuffers req;
    CLEAR(req);

    memset(&req, 0, sizeof(req));

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = mem;

    // allocate buffers in device memory
    if (-1 == ioctl(_fhandle, VIDIOC_REQBUFS, &req))
        throw runtime_error("Can't allocate device buffers.");

    return req.count;
}

void V4L2WebcamDriver::freeDeviceBuffers(enum v4l2_memory mem)
{
    struct v4l2_requestbuffers req;
    CLEAR(req);

    memset(&req, 0, sizeof(req));

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = mem;

    // free buffers in device memory
    ioctl(_fhandle, VIDIOC_REQBUFS, &req);
}

void V4L2WebcamDriver::initRead()
{
    try
    {
        if (!(_vCaps.capabilities & V4L2_CAP_READWRITE))
            throw runtime_error("Device does not support reading.");

        buffers = (IOBuffer*)calloc(1, sizeof(*buffers));

        if (!buffers)
            throw runtime_error("Not enough memory.");

        buffers[0].img = pix_alloc(getPalette(), getWidth(), getHeight());

        if (buffers[0].img == 0)
            throw runtime_error("Not enough memory.");

        _bufferSize = pix_size(getPalette(), getWidth(), getHeight());
        buffers[0].length = _bufferSize;

        n_buffers = 1;

    } catch (runtime_error& ex) {

        if (buffers) {
            if (buffers[0].img) {
                pix_free(buffers[0].img);
                buffers[0].img = NULL;
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }

        throw;
    }
}

void V4L2WebcamDriver::initMmap()
{
    unsigned int count = 0;

    try
    {
        if (!(_vCaps.capabilities & V4L2_CAP_STREAMING))
            throw runtime_error("Device does not support streaming.");

        count = reqDeviceBuffers(V4L2_MEMORY_MMAP, 4);

        if (count < 2)
            throw runtime_error("Not enough device buffers allocated.");

        buffers = (IOBuffer*)calloc(count, sizeof(*buffers));

        if (!buffers)
            throw runtime_error("Not enough memory.");

        for (n_buffers = 0; n_buffers < count; ++n_buffers) {
            struct v4l2_buffer buf;
            CLEAR(buf);

            memset(&buf, 0, sizeof(buf));

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = n_buffers;

            if (-1 == ioctl(_fhandle, VIDIOC_QUERYBUF, &buf))
                throw runtime_error("Error quering device buffer.");

            buffers[n_buffers].img = (piximage*)malloc(sizeof(piximage));

            if (!buffers[n_buffers].img)
                throw runtime_error("Not enough memory.");

            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].img->width = getWidth();
            buffers[n_buffers].img->height = getHeight();
            buffers[n_buffers].img->palette = getPalette();

            buffers[n_buffers].img->data =
                (uint8_t*)mmap(NULL /* start anywhere */,
                      buf.length,
                      PROT_READ | PROT_WRITE /* required */,
                      MAP_SHARED /* recommended */,
                      _fhandle, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].img->data)
                throw runtime_error("Can't mmap device memory.");

            // send to queue
            if (-1 == ioctl(_fhandle, VIDIOC_QBUF, &buf))
                throw runtime_error("Can't enqueue buffer.");
        }
    } catch (runtime_error& er) {
        // free device memory
        freeDeviceBuffers(V4L2_MEMORY_MMAP);

        if (count && buffers)
        {
            for (unsigned int n = 0; n < count; n++) {
                if (buffers[n].img) {
                    if (buffers[n].img->data != MAP_FAILED)
                         munmap(buffers[n].img->data, buffers[n].length);

                     buffers[n].img->data = NULL;

                     free(buffers[n].img);
                     buffers[n].img = NULL;
                }
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }

        throw;
    }
}

void V4L2WebcamDriver::initUserp()
{
    unsigned int count = 0;

    try
    {
        if (!(_vCaps.capabilities & V4L2_CAP_STREAMING))
            throw runtime_error("Device does not support streaming.");

        count = reqDeviceBuffers(V4L2_MEMORY_USERPTR, 4);

        if (count < 2)
            throw runtime_error("Not enough device buffers allocated.");

        buffers = (IOBuffer*)calloc(count, sizeof(*buffers));

        if (!buffers)
            throw runtime_error("Not enough memory.");

        for (n_buffers = 0; n_buffers < 4; ++n_buffers)
        {
            buffers[n_buffers].img = pix_alloc(getPalette(), getWidth(), getHeight());

            if (!buffers[n_buffers].img)
                throw runtime_error("Not enough memory.");

            buffers[n_buffers].length = pix_size(getPalette(), getWidth(), getHeight());

            struct v4l2_buffer buf;
            CLEAR(buf);

            memset(&buf, 0, sizeof(buf));

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.m.userptr = (unsigned long) buffers[n_buffers].img->data;
            buf.length = buffers[n_buffers].length;

            if (-1 == ioctl(_fhandle, VIDIOC_QBUF, &buf))
                throw runtime_error("Can't enqueue buffer.");
        }
    } catch (runtime_error& er) {
        // free device memory
        freeDeviceBuffers(V4L2_MEMORY_USERPTR);

        if (count && buffers) {
            for (unsigned int n = 0; n < count; n++) {
                if (buffers[n].img) {
                    pix_free(buffers[0].img);

                    buffers[n].img = NULL;
                }
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }

        throw;
    }
}

void V4L2WebcamDriver::initDevice()
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    CLEAR(cropcap);
    CLEAR(crop);

    /* Select video input, video standard and tune here. */
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == ioctl(_fhandle, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == ioctl(_fhandle, VIDIOC_S_CROP, &crop)) {
                switch (errno) {
                case EINVAL:
                        /* Cropping not supported. */
                        break;
                default:
                        /* Errors ignored. */
                        break;
                }
        }
    } else {
            /* Errors ignored. */
    }

    try {
        initMmap();
        ioMethod = IO_METHOD_MMAP;

        LOG_DEBUG("Choosen MMAP I/O method.");
    } catch (runtime_error& ex1) {
        LOG_WARN("No MMAP I/O method supported. Reason: " + std::string(ex1.what()));

        try
        {
            initUserp();
            ioMethod = IO_METHOD_USERPTR;

            LOG_DEBUG("Choosen USERPTR I/O method.");
        } catch (runtime_error& ex2) {
            LOG_WARN("No USERTR I/O method supported. Reason: " + std::string(ex2.what()));

            try {
                initRead();
                ioMethod = IO_METHOD_READ;

                LOG_DEBUG("Choosen READ I/O method.");
            } catch (runtime_error& ex3) {
                LOG_WARN("No I/O method supported. Reason: " + std::string(ex3.what()));

                ioMethod = IO_METHOD_UNKNOWN;

                throw runtime_error("");
            }
        }
    }
}

void V4L2WebcamDriver::checkDevice() {
  if (isOpen()) {
    try {
      CLEAR(_vPixFormat);

      _vPixFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == ioctl(_fhandle, VIDIOC_G_FMT, &_vPixFormat)) {
        LOG_WARN("VIDIOC_G_FMT failed");
      }
      _vPixFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      _vPixFormat.fmt.pix.width = 1;
      _vPixFormat.fmt.pix.height = 1;
      _vPixFormat.fmt.pix.field = V4L2_FIELD_ANY;

      if (-1 == ioctl(_fhandle, VIDIOC_S_FMT, &_vPixFormat)) {
        LOG_WARN("Detecting minimum resolution with VIDIOC_S_FMT failed");
      } else {
        _minWidth = _vPixFormat.fmt.pix.width;
        _minHeight = _vPixFormat.fmt.pix.height;
      }

      if (-1 == ioctl(_fhandle, VIDIOC_G_FMT, &_vPixFormat)) {
        LOG_WARN("VIDIOC_G_FMT failed detecting minimum resolution");
      }

      _vPixFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      _vPixFormat.fmt.pix.width = 32767;
      _vPixFormat.fmt.pix.height = 32767;
      _vPixFormat.fmt.pix.field = V4L2_FIELD_ANY;

      if (-1 == ioctl(_fhandle, VIDIOC_S_FMT, &_vPixFormat)) {
        LOG_WARN("Detecting maximum resolution with VIDIOC_S_FMT failed");
      } else {
        _maxWidth = _vPixFormat.fmt.pix.width;
        _maxHeight = _vPixFormat.fmt.pix.height;
      }

      if (-1 == ioctl(_fhandle, VIDIOC_G_FMT, &_vPixFormat)) {
        LOG_WARN("VIDIOC_G_FMT failed detecting maximum resolution");
      }

      /* Buggy driver paranoia ;) */
      unsigned int n;
      struct v4l2_fmtdesc fmtdesc;

      /* format enumeration */
      for (n = 0; ; n++) {
        fmtdesc.index = n;
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(_fhandle, VIDIOC_ENUM_FMT, &fmtdesc) < 0) {
          if (errno == EINVAL) {
            break; /* end of enumeration */
          } else {
            LOG_WARN("Failed to get number " + String::fromNumber(n)  + " in pixelformat enumeration");
          }
        }
        /* We have a format */
        if (pix_v4l2_to_pix_osi(fmtdesc.pixelformat) != PIX_OSI_UNSUPPORTED) {
          _vPixFormat.fmt.pix.pixelformat = fmtdesc.pixelformat;
          if (ioctl(_fhandle, VIDIOC_S_FMT, &_vPixFormat) < 0) {
            LOG_WARN("Failed to set pixelformat: " + std::string(pix_get_fmt_name(pix_v4l2_to_pix_osi(fmtdesc.pixelformat))));
          } else {
            if (ioctl(_fhandle, VIDIOC_G_FMT, &_vPixFormat) < 0) {
              LOG_WARN("VIDIOC_G_FMT failed setting pixelformat");
            } else {
              break; /* end of enumeration */
            }
          }
        }
      }

    } catch (runtime_error& ex) {
      LOG_WARN("Checking device failed: " + std::string(ex.what()));
      throw;
    }
  }
}

void V4L2WebcamDriver::uninitDevice()
{
    switch (ioMethod) {
    case IO_METHOD_READ:
        if (buffers) {
            if (buffers[0].img) {
                pix_free(buffers[0].img);
                buffers[0].img = NULL;
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }
        break;

    case IO_METHOD_MMAP:
        freeDeviceBuffers(V4L2_MEMORY_MMAP);

        if (n_buffers && buffers) {
            for (unsigned int n = 0; n < n_buffers; n++) {
                if (buffers[n].img) {
                    if (buffers[n].img->data)
                         munmap(buffers[n].img->data, buffers[n].length);

                     buffers[n].img->data = NULL;

                     free(buffers[n].img);
                     buffers[n].img = NULL;
                }
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }
        break;

    case IO_METHOD_USERPTR:
        freeDeviceBuffers(V4L2_MEMORY_USERPTR);

        if (n_buffers && buffers) {
            for (unsigned int n = 0; n < n_buffers; n++) {
                if (buffers[n].img) {
                    pix_free(buffers[0].img);

                    buffers[n].img = NULL;
                }
            }
            free(buffers);
            buffers = NULL;
            n_buffers = 0;
        }
        break;
    default:
        break;
    }
}

void V4L2WebcamDriver::startCapture() {
    if (_oldApiDevice)
    {
        _oldApiDriver.startCapture();
        return;
    }

    try
    {
        initDevice();

        enum v4l2_buf_type type;

        switch (ioMethod) {
        case IO_METHOD_UNKNOWN:
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == ioctl(_fhandle, VIDIOC_STREAMON, &type))
                throw runtime_error("Failed starting video stream.");
            break;

        }

        start();

    } catch (runtime_error& ex) {
        LOG_ERROR("Failed starting capture:" + std::string(ex.what()));

        //FIXME maybe we should call uninitDevice()? Are we sure that stopCapture will be called?
    }
}

void V4L2WebcamDriver::pauseCapture() {
        //TODO: implement pause
}

void V4L2WebcamDriver::stopCapture() {
    if (_oldApiDevice)
    {
        _oldApiDriver.stopCapture();
        return;
    }

    terminate();

    enum v4l2_buf_type type;

    switch (ioMethod) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == ioctl(_fhandle, VIDIOC_STREAMOFF, &type))
            return;

        break;
    default:
        break;
    }

    uninitDevice();
}

WebcamErrorCode V4L2WebcamDriver::setPalette(pixosi palette) {
    if (_oldApiDevice)
        return _oldApiDriver.setPalette(palette);

    unsigned int pixelformat = pix_v4l2_from_pix_osi(palette);

    _vPixFormat.fmt.pix.pixelformat = pixelformat;

    LOG_DEBUG("Trying to change webcam pixelformat to " + std::string(pix_get_fmt_name(palette)) + " (" + String::fromNumber(palette) + ")");
    int r = ioctl(_fhandle, VIDIOC_S_FMT, &_vPixFormat);

    if (-1 == r)
        LOG_WARN("Failed settings pixel format.");

    try { readCaps(); } catch (runtime_error& er) { return WEBCAM_NOK; }

    LOG_DEBUG("Webcam pixelformat is " + std::string(pix_get_fmt_name(pix_v4l2_to_pix_osi(_vPixFormat.fmt.pix.pixelformat))) + " (" + String::fromNumber(pix_v4l2_to_pix_osi(_vPixFormat.fmt.pix.pixelformat)) + ")");

    return (r == 0 && _vPixFormat.fmt.pix.pixelformat == pixelformat ? WEBCAM_OK : WEBCAM_NOK);
}

pixosi V4L2WebcamDriver::getPalette() const {
    if (_oldApiDevice)
        return _oldApiDriver.getPalette();

    return pix_v4l2_to_pix_osi(_vPixFormat.fmt.pix.pixelformat);
}

WebcamErrorCode V4L2WebcamDriver::setFPS(unsigned fps) {
    if (_oldApiDevice)
        return _oldApiDriver.setFPS(fps);

    struct v4l2_streamparm setfps;
    CLEAR(setfps);
    memset(&setfps, 0, sizeof(struct v4l2_streamparm));
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.numerator = 1;
    setfps.parm.capture.timeperframe.denominator = fps;
    if (ioctl(_fhandle, VIDIOC_S_PARM, setfps) == -1) {
      return WEBCAM_NOK;
    }
    _fps = fps;
    return WEBCAM_OK;
}

unsigned V4L2WebcamDriver::getFPS() const {
    if (_oldApiDevice)
        return _oldApiDriver.getFPS();

    return _fps;
}

WebcamErrorCode V4L2WebcamDriver::setResolution(unsigned int width, unsigned int height) {
    if (_oldApiDevice)
        return _oldApiDriver.setResolution(width, height);

    if (width > _maxWidth) width = _maxWidth;
    if (height > _maxHeight) height = _maxHeight;
    if (width < _minWidth) width = _minWidth;
    if (height < _minHeight) height = _minHeight;

    _vPixFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    _vPixFormat.fmt.pix.width = width;
    _vPixFormat.fmt.pix.height = height;
    _vPixFormat.fmt.pix.field = V4L2_FIELD_ANY;

    int r = ioctl(_fhandle, VIDIOC_S_FMT, &_vPixFormat);

    if (-1 == r) {
        LOG_WARN("Failed settings picture dimensions.");
    } else {
      /* Buggy driver paranoia. */
      unsigned int min = _vPixFormat.fmt.pix.width * 2;
      if (_vPixFormat.fmt.pix.bytesperline < min) {
        _vPixFormat.fmt.pix.bytesperline = min;
      }
      min = _vPixFormat.fmt.pix.bytesperline * _vPixFormat.fmt.pix.height;
      if (_vPixFormat.fmt.pix.sizeimage < min) {
        _vPixFormat.fmt.pix.sizeimage = min;
      }

      _bufferSize = _vPixFormat.fmt.pix.sizeimage;
    }

    /* Some webcams need the framerate to be set directly after the resolution */
    if (setFPS(15) == WEBCAM_NOK) {
      return WEBCAM_NOK;
    }

    try { readCaps(); } catch (runtime_error& er) { return WEBCAM_NOK; }

    return r == 0 ? WEBCAM_OK : WEBCAM_NOK;
}

unsigned V4L2WebcamDriver::getWidth() const {
    if (_oldApiDevice)
        return _oldApiDriver.getWidth();

    return _vPixFormat.fmt.pix.width;
}

unsigned V4L2WebcamDriver::getHeight() const {
    if (_oldApiDevice)
        return _oldApiDriver.getHeight();

    return _vPixFormat.fmt.pix.height;;
}

void V4L2WebcamDriver::setBrightness(int brightness) {
    if (_oldApiDevice)
        return _oldApiDriver.setBrightness(brightness);
}

int V4L2WebcamDriver::getBrightness() const {
    if (_oldApiDevice)
        return _oldApiDriver.getBrightness();

    return 0;
}

void V4L2WebcamDriver::setContrast(int contrast) {
    if (_oldApiDevice)
        return _oldApiDriver.setContrast(contrast);

}

int V4L2WebcamDriver::getContrast() const {
    if (_oldApiDevice)
        return _oldApiDriver.getContrast();

    return 0;
}

void V4L2WebcamDriver::flipHorizontally(bool flip) {
    if (_oldApiDevice)
    {
        _oldApiDriver.flipHorizontally(flip);
        return;
    }

    //TODO: add horizontal flip support
}

void V4L2WebcamDriver::readCaps() {
    if (isOpen()) {
        try {
            if (-1 == ioctl(_fhandle, VIDIOC_QUERYCAP, &_vCaps))
                throw runtime_error("No V4L2 device");

            if (-1 == ioctl(_fhandle, VIDIOC_G_FMT, &_vPixFormat))
                throw runtime_error("Getting the format of data failed");

        } catch (runtime_error& ex) {
            LOG_WARN("Reading capabilities failed. " + std::string(ex.what()));
            throw;
        }
    }
}

WebcamErrorCode V4L2WebcamDriver::readFrame()
{
    struct v4l2_buffer buf;
    unsigned int i;
    CLEAR(buf);

    switch (ioMethod)        {
    case IO_METHOD_READ:
        if (-1 == read(_fhandle, buffers[0].img->data, buffers[0].length)) {
            switch (errno) {
            case EAGAIN:
               return WEBCAM_OK;
            default:
               return WEBCAM_NOK;
            }
        }

        _webcamDriver->frameBufferAvailable(buffers[0].img);

        break;

    case IO_METHOD_MMAP:
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(_fhandle, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                    return WEBCAM_OK;
            default:
                    return WEBCAM_NOK;
            }
        }

        assert(buf.index < n_buffers);

        _webcamDriver->frameBufferAvailable(buffers[buf.index].img);

        if (-1 == ioctl(_fhandle, VIDIOC_QBUF, &buf))
                return WEBCAM_OK;

        break;

    case IO_METHOD_USERPTR:
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (-1 == ioctl(_fhandle, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return WEBCAM_OK;
            default:
                return WEBCAM_NOK;
            }
        }

        for (i = 0; i < n_buffers; ++i)
            if (buf.m.userptr == (unsigned long) buffers[i].img->data
                && buf.length == buffers[i].length)
                break;

        assert(i < n_buffers);

        _webcamDriver->frameBufferAvailable(buffers[i].img);

        if (-1 == ioctl(_fhandle, VIDIOC_QBUF, &buf))
            return WEBCAM_NOK;

        break;
    default:
        break;
    }

    return WEBCAM_OK;
}

void V4L2WebcamDriver::run() {
    if (_oldApiDevice)
    {
        _oldApiDriver.run();
        return;
    }

    float st,et;
    float frameInter = 1000 / (float)_fps;

    while (isOpen() && !_terminate) {
#ifdef CC_MSVC
        st = timeGetTime();
#else
        st = clock() / ((float)CLOCKS_PER_SEC / 1000.0);
#endif

        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(_fhandle, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(_fhandle + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
	        if (EINTR == errno)
	            continue;
	
	        break;
        }

        if (0 == r) /* timeout */
            break;

        if (!isOpen())
            break;

        if (readFrame() != WEBCAM_OK)
            break;

#ifdef CC_MSVC
        et = timeGetTime();
#else
        et = clock() / ((float)CLOCKS_PER_SEC / 1000.0);
#endif
        if (et-st < frameInter)
            msleep((long unsigned int)(frameInter - et + st));
    }
}

V4L2WebcamDriver::DevNameArray V4L2WebcamDriver::getDevices() {
    DevNameArray v4l2Devs = getDevices2_6();

    V4LWebcamDriver::DevNameArray v4lDevs = _oldApiDriver.getDevices();

    DevNameArray::const_iterator i = v4lDevs.begin();

    while (i != v4lDevs.end()) {
        if (v4l2Devs.find((*i).first) == v4l2Devs.end())
            v4l2Devs[(*i).first] = (*i).second;
        i++;
    }

    return v4l2Devs;
}

V4L2WebcamDriver::DevNameArray V4L2WebcamDriver::getDevices2_6() {
        const string dir = "/sys/class/video4linux";
        DevNameArray array;
        File sysDir(dir);
        StringList list = sysDir.getDirectoryList();
        struct v4l2_capability caps;
        CLEAR(caps);

        for (register unsigned i = 0 ; i < list.size() ; i++) {
            if (list[i][0] != '.') {
                int fd = open(("/dev/" + list[i]).c_str(), O_RDWR | O_NONBLOCK);

                if (fd <= 0)
                    continue;

                int r = ioctl(fd, VIDIOC_QUERYCAP, &caps);

                close(fd);

                if (-1 == r)
                    continue;

                if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE))
                    continue;

                if (!(caps.capabilities & (V4L2_CAP_READWRITE | V4L2_CAP_STREAMING)))
                    continue;

                ifstream nameFile((dir + "/" + list[i] + "/name").c_str());
                char buffer[512];
                nameFile.getline(buffer, 512);

                strncat(buffer, (" : " + list[i]).c_str(), list[i].size() + 3);

                array[list[i]] = buffer;
            }
        }

        return array;
}

/* TODO: test this part
V4L2WebcamDriver::DevNameArray V4L2WebcamDriver::getDevices2_4() {
        const string dir = "/proc/video/dev";
        DevNameArray array;
        File sysDir(dir);
        StringList list = sysDir.getDirectoryList();

        for (register unsigned i = 0 ; i < list.size() ; i++) {
                if (list[i][0] != '.') {
                        ifstream nameFile((dir + list[i]).c_str());
                        char buffer[512];
                        nameFile.getline(buffer, 512);
                        char *colonPos = strchr(buffer, ':');

                        array[list[i]] = colonPos + 2;
                }
        }

        return array;
}
*/
