/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#include <webcam/DirectXWebcamDriver.h>

#include <webcam/DirectXWebcamUtils.h>

#include <pixertool/directx-pixertool.h>

#include <util/Logger.h>

using namespace std;

#define SAFE_RELEASE_POINTER(x) { if (x) x->Release(); x = NULL; }
#define SAFE_RELEASE(x) { if (x) x.Release(); x = NULL; }

DirectXWebcamDriver::DirectXWebcamDriver(WebcamDriver * driver, int flags)
	: IWebcamDriver(flags) {

	// no more needed since apiInitialize() was introduced.
	//FIXME does not work because of Qt4.1.2
	//CoInitializeEx(NULL, COINIT_MULTITHREADED);
	////
	//CoInitialize(NULL);
	////

	_webcamDriver = driver;
	_pGrabberF = NULL;
	_pGrabber = NULL;
	_pNull = NULL;
	_iam = NULL;
	_pGraph = NULL;
	_pControl = NULL;
	_pCap = NULL;
	_pBuild = NULL;
}

DirectXWebcamDriver::~DirectXWebcamDriver() {
	stopCapture();
	// no more needed since apiUninitialize() was introduced.
	//CoUninitialize();
	////
}

void DirectXWebcamDriver::cleanup() {
	LOG_DEBUG("Cleaning up the Direct X webcam driver");
	_isOpen = false;
	_cachedWidth = 0;
	_cachedHeight = 0;
	_cachedFPS = 15;
	_cachedPalette = PIX_OSI_UNSUPPORTED;
	SAFE_RELEASE_POINTER(_pGrabberF);
	SAFE_RELEASE_POINTER(_pGrabber);
	SAFE_RELEASE_POINTER(_pNull);
	SAFE_RELEASE_POINTER(_iam);
	SAFE_RELEASE(_pControl);
	SAFE_RELEASE(_pGraph);
	SAFE_RELEASE(_pCap);
	SAFE_RELEASE(_pBuild);
}

StringList DirectXWebcamDriver::getDeviceList() {
	StringList deviceList;

	LOG_DEBUG("Direct X Get Device List");

	//create an enumerator
	CComPtr< ICreateDevEnum > pCreateDevEnum;
	pCreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (!pCreateDevEnum) {
		return deviceList;
	}

	//enumerate video capture devices
	CComPtr< IEnumMoniker > pEnumMoniker;
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	if (!pEnumMoniker) {
		return deviceList;
	}

	pEnumMoniker->Reset();
	//go through and find all video capture device(s)
	while (true) {
		CComPtr< IMoniker > pMoniker;
		HRESULT hr = pEnumMoniker->Next(1, &pMoniker, 0);
		if (hr != S_OK) {
			break;
		}

		//get the property bag for this moniker
		CComPtr< IPropertyBag > pPropertyBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**) &pPropertyBag);
		if (hr != S_OK) {
			continue;
		}

		//ask for the english-readable name
		CComVariant FriendlyName;
		CComVariant DevicePath;
		hr = pPropertyBag->Read(L"FriendlyName", &FriendlyName, NULL);
		hr = pPropertyBag->Read(L"DevicePath", &DevicePath, NULL);
		if (hr != S_OK) {
			continue;
		}

		std::string deviceName;
		if (((string) _bstr_t(DevicePath)).find("pci") == string::npos) {
			deviceName = (string) _bstr_t(FriendlyName);
			deviceList += deviceName;
		}

		/* TODO: do we still use this variable? see lib video in classic.
		else if (pci_device) {
			deviceName = (string) _bstr_t(FriendlyName);
			deviceList += deviceName;
		}
		*/
	}

	return deviceList;
}

std::string DirectXWebcamDriver::getDefaultDevice() {
	std::string defaultDevice;

	LOG_DEBUG("Direct X Get Default Device");

	//create an enumerator
	CComPtr< ICreateDevEnum > pCreateDevEnum;
	pCreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);

	//_ASSERTE(pCreateDevEnum);
	if (!pCreateDevEnum) {
		return WEBCAM_NULL;
	}

	//enumerate video capture devices
	CComPtr< IEnumMoniker > pEm;
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

	//_ASSERTE(pEm);
	if (!pEm) {
		return WEBCAM_NULL;
	}

	pEm->Reset();

	//go through and find first video capture device
	while (true) {
		ULONG ulFetched = 0;
		CComPtr< IMoniker > pM;

		HRESULT hr = pEm->Next(1, &pM, &ulFetched);
		if (hr != S_OK) {
			break;
		}

		//get the property bag
		CComPtr< IPropertyBag > pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**) &pBag);
		if (hr != S_OK) {
			continue;
		}

		//ask for the english-readable name
		CComVariant var;
		var.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &var, NULL);
		if (hr != S_OK) {
			continue;
		}

		defaultDevice = (const char *) _bstr_t(var);

		//ask for the actual filter
		CComPtr< IBaseFilter > ppCap;
		hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**) &ppCap);
		if (ppCap) {
			break;
		}
	}

	return defaultDevice;
}

WebcamErrorCode DirectXWebcamDriver::setDevice(const std::string & deviceName) {
	//TODO: test if a webcam is already open

	//TODO: refactor the COM initialization phase to avoid
	//multiple initalisations and better handle unitialization
	//cf trac ticket #1008

	// We really need to refactor that point
	// I leave this line here just because the phapi thread
	// must call this function (one time). We must move this elsewhere ...
	CoInitialize(NULL);
	////

	_pGraph.CoCreateInstance(CLSID_FilterGraph);
	if (!_pGraph) {
		LOG_ERROR("failed to create Graph builder");
		return WEBCAM_NOK;
	}

	_pBuild.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	if (!_pBuild) {
		LOG_ERROR("failed to create Capture Graph builder");
		return WEBCAM_NOK;
	}

	_pBuild->SetFiltergraph(_pGraph);
	//Create the Sample Grabber
	HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(_pGrabberF));
	if (hr != S_OK) {
		LOG_ERROR("failed to create COM instance");
		return WEBCAM_NOK;
	}

	//Add the filter to the graph
	hr = (_pGraph)->AddFilter(_pGrabberF, L"Sample Grabber");
	if (hr != S_OK) {
		LOG_ERROR("failed to add filter");
		return WEBCAM_NOK;
	}

	//Query the Sample Grabber for the ISampleGrabber interface.
	_pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&_pGrabber);
	hr = _pGrabber->SetBufferSamples(FALSE);
	hr = _pGrabber->SetOneShot(FALSE);

	//Set the Sample Grabber callback
	//0: SampleCB (the buffer is the original buffer, not a copy)
	//1: BufferCB (the buffer is a copy of the original buffer)
	if (_pGrabber->SetCallback(this, 0) != S_OK) {
		LOG_ERROR("failed to assign callback");
		return WEBCAM_NOK;
	}

	CComBSTR bstrName(deviceName.c_str());
	hr = FindMyCaptureDevice(&_pCap, bstrName);
	if ((hr != S_OK) || !_pCap) {
		return WEBCAM_NOK;
	}

	//initialize IAMStreamConfig
	_iam = GetIAMStreamConfig(_pCap);
	if (!_iam) {
		return WEBCAM_NOK;
	}

	//add the capture filter to the graph
	hr = (_pGraph)->AddFilter(_pCap, L"");
	if (hr != S_OK) {
		LOG_ERROR("failed to add filter");
		return WEBCAM_NOK;
	}

	//Add a null renderer filter
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&_pNull);
	hr = (_pGraph)->AddFilter(_pNull, L"NullRender");

	_pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, NULL, _pCap, NULL, _pGrabberF);

	//try to assign some palette until the webcam supports it
	if (setCaps(PIX_OSI_YUV420P, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_YUV420P, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_I420, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_I420, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB32, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB32, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB24, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB24, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_YUV422, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_YUV422, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB565, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB565, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB555, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_RGB555, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_YUY2, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_YUY2, _cachedFPS, 160, 120) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_UYVY, _cachedFPS, 176, 144) != WEBCAM_OK) {
	if (setCaps(PIX_OSI_UYVY, _cachedFPS, 160, 120) != WEBCAM_OK) {
	}}}}}}}}}}}}}}}}}}

	readCaps();

	if (getPalette() == PIX_OSI_UNSUPPORTED) {
		return WEBCAM_NOK;
	}

	_isOpen = true;

	return WEBCAM_OK;
}

bool DirectXWebcamDriver::isOpen() const {
	return _isOpen;
}

void DirectXWebcamDriver::startCapture() {
	HRESULT hr;
	hr = _pGraph->QueryInterface(IID_IMediaControl, (void **)&_pControl);
	if (hr != S_OK) {
		LOG_ERROR("Could not get _pControl MediaControl");
	}

	hr = _pControl->Run();
	if (hr != S_OK) {
		LOG_ERROR("Could not run graph");
		return;
	}

	SAFE_RELEASE(_pControl);
}

void DirectXWebcamDriver::pauseCapture() {
}

void DirectXWebcamDriver::stopCapture() {
	HRESULT hr;

	if (!_pGraph) {
		LOG_WARN("_pGraph is NULL");
		return;
	}

	hr = _pGraph->QueryInterface(IID_IMediaControl, (void **)&_pControl);
	if (hr != S_OK) {
		LOG_ERROR("Could not get _pControl MediaControl");
	}

	hr = _pControl->StopWhenReady();
	if (hr != S_OK) {
		LOG_ERROR("Could not stop capture");
	}

	SAFE_RELEASE(_pControl);
}

WebcamErrorCode DirectXWebcamDriver::setPalette(pixosi palette) {
	setCaps(palette, getFPS(), getWidth(), getHeight());
	if (_cachedPalette != palette) {
		return WEBCAM_NOK;
	} else {
		return WEBCAM_OK;
	}
}

pixosi DirectXWebcamDriver::getPalette() const {
	return _cachedPalette;
}

WebcamErrorCode DirectXWebcamDriver::setFPS(unsigned fps) {
	setCaps(getPalette(), fps, getWidth(), getHeight());

	//TODO: check if fps is correctly set
	return WEBCAM_NOK;
}

unsigned DirectXWebcamDriver::getFPS() const {
	return _cachedFPS;
}

WebcamErrorCode DirectXWebcamDriver::setResolution(unsigned width, unsigned height) {
	setCaps(getPalette(), getFPS(), width, height);
	if ((_cachedWidth != width) || (_cachedHeight != height)) {
		return WEBCAM_NOK;
	} else {
		return WEBCAM_OK;
	}
}


unsigned DirectXWebcamDriver::getWidth() const {
	return _cachedWidth;
}

unsigned DirectXWebcamDriver::getHeight() const {
	return _cachedHeight;
}

void DirectXWebcamDriver::setBrightness(int brightness) {
}

int DirectXWebcamDriver::getBrightness() const {
	return 0;
}

void DirectXWebcamDriver::setContrast(int contrast) {
}

int DirectXWebcamDriver::getContrast() const {
	return 0;
}

void DirectXWebcamDriver::flipHorizontally(bool flip) {
	//TODO: add horizontal flip support
}

WebcamErrorCode DirectXWebcamDriver::setCaps(pixosi palette, unsigned fps, unsigned resolutionWidth, unsigned resolutionHeight) {
	_cachedFPS = fps;

	if (!isOpen()) {
		return WEBCAM_NOK;
	}

	if (!_pCap) {
		LOG_FATAL("webcam not initialized");
	}
	if (!_iam) {
		LOG_FATAL("webcam not initialized");
	}

	int iCount, iSize;
	HRESULT hr = _iam->GetNumberOfCapabilities(&iCount, &iSize);

	VIDEO_STREAM_CONFIG_CAPS scc;
	if (sizeof(scc) != iSize) {
		LOG_ERROR("wrong config structure");
		return WEBCAM_NOK;
	}

	for (int i = 0; i < iCount; i++) {
		AM_MEDIA_TYPE * pmt = NULL;
		hr = _iam->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE *>(&scc));
		if (hr == S_OK) {
			pixosi wc_palette = pix_directx_to_pix_osi(pmt->subtype);
			if (wc_palette != palette) {
				hr = E_FAIL;
				continue;
			}
			VIDEOINFOHEADER * pvi = (VIDEOINFOHEADER *) pmt->pbFormat;
			pvi->bmiHeader.biWidth = resolutionWidth;
			pvi->bmiHeader.biHeight = resolutionHeight;
			pvi->AvgTimePerFrame = (LONGLONG) (10000000. / (double)fps);
			hr = _iam->SetFormat(pmt);
			if (hr != S_OK) {
				hr = E_FAIL;
				continue;
			} else {
				LOG_DEBUG("assigned caps : ("
					+ String::fromNumber(palette)
					+ "," + String::fromNumber(fps)
					+ "," + String::fromNumber(resolutionWidth)
					+ "," + String::fromNumber(resolutionHeight)
					+ ")");
				break;
			}
		}
	}

	readCaps();

	if (hr == S_OK) {
		return WEBCAM_OK;
	}

	LOG_ERROR("failed caps request: ("
		+ String::fromNumber(palette)
		+ "," + String::fromNumber(fps)
		+ "," + String::fromNumber(resolutionWidth)
		+ "," + String::fromNumber(resolutionHeight)
		+ ")");

	return WEBCAM_NOK;
}

void DirectXWebcamDriver::readCaps() {
	VIDEOINFOHEADER * pvi;
	pixosi palette;

	AM_MEDIA_TYPE * pmt = NULL;
	HRESULT hr = _iam->GetFormat(&pmt);

	if (pmt->formattype == FORMAT_VideoInfo) {
		pvi = (VIDEOINFOHEADER *) pmt->pbFormat;
		palette = pix_directx_to_pix_osi(pmt->subtype);
	}

	_cachedPalette = palette;
	_cachedWidth = pvi->bmiHeader.biWidth;
	_cachedHeight = pvi->bmiHeader.biHeight;
}

STDMETHODIMP DirectXWebcamDriver::QueryInterface(REFIID riid, void ** ppv) {
	LOG_DEBUG("CSampleGrabberCB::QueryInterface");

	if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
		*ppv = (void *) static_cast<ISampleGrabberCB*> (this);
		return NOERROR;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP DirectXWebcamDriver::BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize) {
	//This method is not used but must be implemented
	LOG_ERROR("this method should not be called");
	return 0;
}

STDMETHODIMP DirectXWebcamDriver::SampleCB(double SampleTime, IMediaSample * pSample) {
	if (!pSample) {
		return E_POINTER;
	} else {
		BYTE * pBuffer;
		pSample->GetPointer(&pBuffer);
		long lBufferSize = pSample->GetSize();

		if (!pBuffer) {
			return E_POINTER;
		}

		_capturedImage.data = (uint8_t *) pBuffer;
		_capturedImage.width = _cachedWidth;
		_capturedImage.height = _cachedHeight;
		_capturedImage.palette = _cachedPalette;

		_webcamDriver->frameBufferAvailable(&_capturedImage);
	}
	return 0;
}
