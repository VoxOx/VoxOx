/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include <webcam/DirectXWebcamUtils.h>

#include <iostream>
using namespace std;

HRESULT FindMyCaptureDevice(IBaseFilter * * pF, BSTR bstrName) {
	HRESULT hr = E_FAIL;
	CComPtr < IBaseFilter > pFilter;
	CComPtr < ICreateDevEnum > pSysDevEnum;
	CComPtr < IEnumMoniker > pEnumCat = NULL;

	// Create the System Device Enumerator.
	pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (!pSysDevEnum) {
		return E_FAIL;
	}

	// Obtain a class enumerator for the video compressor category.
	pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, & pEnumCat, 0);
	if (!pEnumCat) {
		return E_FAIL;
	}

	pEnumCat->Reset();

	// Enumerate the monikers.
	//CComPtr<IMoniker> pMoniker;

	while (true) {
		//while(pMoniker && pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		CComPtr < IMoniker > pMoniker;
		ULONG cFetched;
		CComPtr < IPropertyBag > pProp;

		HRESULT hr_work = pEnumCat->Next(1, & pMoniker, & cFetched);
		if (hr_work != S_OK) {
			break;
		}

		hr = pMoniker->BindToStorage(0, 0,
		IID_IPropertyBag, (void * *) & pProp);

		if (hr != S_OK) {
			continue;
		}

		VARIANT varName;
		VariantInit(& varName); // Try to match the friendly name.
		hr = pProp->Read(L"FriendlyName", & varName, 0);
		if (SUCCEEDED(hr) && (wcscmp(bstrName, varName.bstrVal) == 0)) {
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void * *) & pFilter);
			break;
		}
		VariantClear(& varName);
		pMoniker = NULL; // Release for the next loop.
	}
	if (pFilter) {
		*pF = pFilter;
		(*pF)->AddRef(); // Add ref on the way out.
	}
	return hr;
}

IAMStreamConfig * GetIAMStreamConfig(IBaseFilter * pFilter) {
	IEnumPins * pEnum = NULL;

	HRESULT hr = pFilter->EnumPins(& pEnum);
	if (FAILED(hr)) {
		return NULL;
	}

	IPin * pPin = NULL;
	while (pEnum->Next(1, & pPin, NULL) == S_OK) {
		IAMStreamConfig * pIAMS = NULL;
		hr = pPin->QueryInterface(IID_IAMStreamConfig, (void * *) & pIAMS);
		if (hr == S_OK) {
			return pIAMS;
		}
		pPin->Release();
	}
	pEnum->Release();
	return NULL;
}
