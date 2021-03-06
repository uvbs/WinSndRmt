// Epvolume.h -- Implementation of IAudioEndpointVolumeCallback interface

#include <windows.h>
//#include <commctrl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
//#include "resource.h"

// Dialog handle from dialog box procedure
//extern HWND g_hDlg;

// Client's proprietary event-context GUID
extern GUID g_guidMyContext;

// Maximum volume level on trackbar
#define MAX_VOL  100

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

//-----------------------------------------------------------
// Client implementation of IAudioEndpointVolumeCallback
// interface. When a method in the IAudioEndpointVolume
// interface changes the volume level or muting state of the
// endpoint device, the change initiates a call to the
// client's IAudioEndpointVolumeCallback::OnNotify method.
//-----------------------------------------------------------
class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
{
    LONG _cRef;
	void sndVolumeOnNetwork(int volume, BOOL bMute);
	void *ntwrkClsRef;

public:
    CAudioEndpointVolumeCallback() :
        _cRef(1), ntwrkClsRef(NULL)
    {
    }

    ~CAudioEndpointVolumeCallback()
    {
    }

    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;

    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
        {
            AddRef();
            *ppvInterface = (IAudioEndpointVolumeCallback*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // Callback method for endpoint-volume-change notifications.

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
    {
        if (pNotify == NULL)
        {
            return E_INVALIDARG;
        }
		char outTextBuff[200];
		if(pNotify->guidEventContext != g_guidMyContext){
			sprintf_s(outTextBuff, "On your Notify %f %s\n", pNotify->fMasterVolume, pNotify->bMuted?"mute":"unmute");
			sndVolumeOnNetwork((int)(MAX_VOL*pNotify->fMasterVolume + 0.5), pNotify->bMuted);
		}
		else
			sprintf_s(outTextBuff, "On my Notify %f %s\n", pNotify->fMasterVolume, pNotify->bMuted?"mute":"unmute");
		OutputDebugString(outTextBuff);
		
        //if (g_hDlg != NULL && pNotify->guidEventContext != g_guidMyContext)
        //{
            //PostMessage(GetDlgItem(g_hDlg, IDC_CHECK_MUTE), BM_SETCHECK,
            //            (pNotify->bMuted) ? BST_CHECKED : BST_UNCHECKED, 0);

            //PostMessage(GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME),
            //            TBM_SETPOS, TRUE,
            //            LPARAM((UINT32)(MAX_VOL*pNotify->fMasterVolume + 0.5)));
        //}
        return S_OK;
    }
	void setNtwrkRef(void *ntwrkCls){
		ntwrkClsRef = ntwrkCls;
	}
};
