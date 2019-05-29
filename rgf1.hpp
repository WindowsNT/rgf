
#pragma once
#pragma comment(lib, "comsupp.lib")

namespace RGF
{



	// RKEY
		// RKEY, quick registry access 
	class RKEY
	{
	private:
		HKEY k = 0;
	public:


		class VALUE
		{
		public:
			std::wstring name;
			std::vector<char> value; // For enums
			HKEY k = 0;
			mutable DWORD ty = 0;

			VALUE(const wchar_t* s, HKEY kk)
			{
				if (s)
					name = s;
				k = kk;
			}

			bool operator =(const wchar_t* val)
			{
				ty = REG_SZ;
				if (RegSetValueEx(k, name.c_str(), 0, REG_SZ, (BYTE*)val, (DWORD)(wcslen(val) * sizeof(wchar_t))) == ERROR_SUCCESS)
					return true;
				return false;
			}
			bool operator =(unsigned long val)
			{
				ty = REG_DWORD;
				return RegSetValueEx(k, name.c_str(), 0, REG_DWORD, (BYTE*)& val, sizeof(val)) == ERROR_SUCCESS;
			}
			bool operator =(unsigned long long val)
			{
				ty = REG_QWORD;
				return RegSetValueEx(k, name.c_str(), 0, REG_QWORD, (BYTE*)& val, sizeof(val)) == ERROR_SUCCESS;
			}


			bool Exists()
			{
				DWORD ch = 0;
				if (RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch) == ERROR_FILE_NOT_FOUND)
					return false;
				return true;
			}

			template <typename T>
			operator T() const
			{
				T ch = 0;
				RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
				std::vector<char> d(ch + 10);
				ch += 10;
				RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
				T ret = 0;
				memcpy(&ret, d.data(), sizeof(T));
				return ret;
			}

			operator std::wstring() const
			{
				DWORD ch = 0;
				RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
				std::vector<char> d(ch + 10);
				ch += 10;
				RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
				return std::wstring((const wchar_t*)d.data());
			}

			bool Delete()
			{
				return (RegDeleteValue(k, name.c_str()) == ERROR_SUCCESS);
			}




		};



		RKEY(HKEY kk)
		{
			k = kk;
		}


		RKEY(const RKEY & k)
		{
			operator =(k);
		}
		void operator =(const RKEY & r)
		{
			Close();
			DuplicateHandle(GetCurrentProcess(), r.k, GetCurrentProcess(), (LPHANDLE)& k, 0, false, DUPLICATE_SAME_ACCESS);
		}

		RKEY(RKEY && k)
		{
			operator =(std::forward<RKEY>(k));
		}
		void operator =(RKEY && r)
		{
			Close();
			k = r.k;
			r.k = 0;
		}

		void operator =(HKEY kk)
		{
			Close();
			k = kk;
		}

		RKEY(HKEY root, const wchar_t* subkey, DWORD acc = KEY_ALL_ACCESS, bool Op = false)
		{
			Load(root, subkey, acc, Op);
		}
		bool Load(HKEY root, const wchar_t* subkey, DWORD acc = KEY_ALL_ACCESS, bool Op = false)
		{
			Close();
			if (Op)
				return (RegOpenKeyEx(root, subkey, 0, acc, &k) == ERROR_SUCCESS);
			return (RegCreateKeyEx(root, subkey, 0, 0, 0, acc, 0, &k, 0) == ERROR_SUCCESS);
		}

		void Close()
		{
			if (k)
				RegCloseKey(k);
			k = 0;
		}

		~RKEY()
		{
			Close();
		}

		bool Valid() const
		{
			if (k)
				return true;
			return false;
		}

		bool DeleteSingle(const wchar_t* sub)
		{
			return (RegDeleteKey(k, sub) == ERROR_SUCCESS);
		}

		bool Delete(const wchar_t* sub = 0)
		{
#if _WIN32_WINNT >= 0x600
			return (RegDeleteTree(k, sub) == ERROR_SUCCESS);
#else
			return false;
#endif
		}

		bool Flush()
		{
			return (RegFlushKey(k) == ERROR_SUCCESS);
		}

		std::vector<std::wstring> EnumSubkeys() const
		{
			std::vector<std::wstring> data;
			for (int i = 0;; i++)
			{
				std::vector<wchar_t> n(300);
				DWORD sz = (DWORD)n.size();
				if (RegEnumKeyEx(k, i, n.data(), &sz, 0, 0, 0, 0) != ERROR_SUCCESS)
					break;
				data.push_back(n.data());
			}
			return data;
		}

		std::vector<VALUE> EnumValues() const
		{
			std::vector<VALUE> data;
			for (int i = 0;; i++)
			{
				std::vector<wchar_t> n(300);
				DWORD sz = (DWORD)n.size();
				DWORD ay = 0;
				RegEnumValue(k, i, n.data(), &sz, 0, 0, 0, &ay);
				std::vector<char> v(ay);
				DWORD ty = 0;
				sz = (DWORD)n.size();
				if (RegEnumValue(k, i, n.data(), &sz, 0, &ty, (LPBYTE)v.data(), &ay) != ERROR_SUCCESS)
					break;

				VALUE x(n.data(), k);
				x.ty = ty;
				x.value = v;
				data.push_back(x);
			}
			return data;
		}

		VALUE operator [](const wchar_t* v) const
		{
			VALUE kv(v, k);
			return kv;
		}

		operator HKEY()
		{
			return k;
		}
	};



	// AX
	namespace AXLIBRARY
	{
#pragma warning(disable:4100)

		// messages
#define AX_QUERYINTERFACE (WM_USER + 1)
#define AX_INPLACE (WM_USER + 2)
#define AX_GETAXINTERFACE (WM_USER + 3)
#define AX_CONNECTOBJECT (WM_USER + 4)
#define AX_DISCONNECTOBJECT (WM_USER + 5)
#define AX_SETDATAADVISE (WM_USER + 6)
#define AX_DOVERB (WM_USER + 7)
#define AX_SETCOMMANDCALLBACK  (WM_USER + 8)
#define AX_SETDISPATCHNOTIFICATION (WM_USER + 9)
// #define AX_SETSERVICEPROVIDER (WM_USER + 10)
#define AX_SETDISPATCHNOTIFICATIONFUNC (WM_USER + 11)
#define AX_RECREATE (WM_USER + 12)
#define AX_RECREATEFROMDATA (WM_USER + 150)
#define AX_SETPARENT (WM_USER + 15)


// notifications
#define AXN_SIZE 1


// Registration function
		ATOM AXRegister();
		int AXConnectObject(IUnknown* Container, REFIID riid, IUnknown* Advisor, IConnectionPointContainer** picpc, IConnectionPoint** picp);
		void AXDisconnectObject(IConnectionPointContainer* icpc, IConnectionPoint* icp, unsigned int Cookie);

		struct AX_CONNECTSTRUCT
		{
			IUnknown* Advisor;
			IConnectionPointContainer* icpc;
			IConnectionPoint* icp;
			DWORD id;
			CLSID SpecialIIDForceOK;
		};


#pragma warning(disable:4584)
		// Class AXClientSide
		class AXClientSite :
			public IOleClientSite,
			public IUnknown,
			public IServiceProvider,
			public IDispatch,
			public IAdviseSink,
			public IOleInPlaceSite,
			public IOleInPlaceFrame
		{
		protected:

			int refNum;

		public:


			HWND Window;
			HWND Parent;
			HMENU Menu;
			bool InPlace;
			int ExternalPlace;
			bool CalledCanInPlace;
			CLSID SpecialIIDForceOK;

			class AX* ax;

			// MyClientSite Methods
			AXClientSite();
			virtual ~AXClientSite();
			STDMETHODIMP_(void) OnDataChange2(FORMATETC*);

			// IUnknown methods
			STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// IServiceProvider methods
			STDMETHODIMP QueryService(REFGUID guid, REFIID iid, void** ppvObject);

			// IOleClientSite methods
			STDMETHODIMP SaveObject();
			STDMETHODIMP GetMoniker(DWORD dwA, DWORD dwW, IMoniker** pm);
			STDMETHODIMP GetContainer(IOleContainer** pc);
			STDMETHODIMP ShowObject();
			STDMETHODIMP OnShowWindow(BOOL f);
			STDMETHODIMP RequestNewObjectLayout();

			// IAdviseSink methods
			STDMETHODIMP_(void) OnDataChange(FORMATETC* pFormatEtc, STGMEDIUM* pStgmed);

			STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lIndex);
			STDMETHODIMP_(void) OnRename(IMoniker* pmk);
			STDMETHODIMP_(void) OnSave();
			STDMETHODIMP_(void) OnClose();

			// IOleInPlaceSite methods
			STDMETHODIMP GetWindow(HWND* p);
			STDMETHODIMP ContextSensitiveHelp(BOOL);
			STDMETHODIMP CanInPlaceActivate();
			STDMETHODIMP OnInPlaceActivate();
			STDMETHODIMP OnUIActivate();
			STDMETHODIMP GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT r1, LPRECT r2, LPOLEINPLACEFRAMEINFO o);
			STDMETHODIMP Scroll(SIZE s);
			STDMETHODIMP OnUIDeactivate(int);
			STDMETHODIMP OnInPlaceDeactivate();
			STDMETHODIMP DiscardUndoState();
			STDMETHODIMP DeactivateAndUndo();
			STDMETHODIMP OnPosRectChange(LPCRECT);

			// IOleInPlaceFrame methods
			STDMETHODIMP GetBorder(LPRECT l);
			STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS);
			STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS w);
			STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject* pV, LPCOLESTR s);
			STDMETHODIMP InsertMenus(HMENU h, LPOLEMENUGROUPWIDTHS x);
			STDMETHODIMP SetMenu(HMENU h, HOLEMENU hO, HWND hw);
			STDMETHODIMP RemoveMenus(HMENU h);
			STDMETHODIMP SetStatusText(LPCOLESTR t);
			STDMETHODIMP EnableModeless(BOOL f);
			STDMETHODIMP TranslateAccelerator(LPMSG, WORD);


			// IDispatch Methods
			HRESULT _stdcall GetTypeInfoCount(unsigned int* pctinfo);
			HRESULT _stdcall GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo);
			HRESULT _stdcall GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR*, unsigned int cNames, LCID lcid, DISPID FAR*);
			HRESULT _stdcall Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr);

			// IOleControlSite Methods
		};




		// Class AX
		class AX
		{
		public:

			friend class AXClientSite;
			AX(char* clsid);
			CLSID GetCLSID();
			~AX();


			void Init(char* clsid);
			void Clean();

			AXClientSite Site;
			IID* iid;

			//_COM_SMARTPTR_TYPEDEF(IOleObject,__uuidof(IOleObject));
			CComPtr<IOleObject> OleObject;
			//		_com_ptr_t<IOleObject> OleObject;
					//_com_ptr_t<IStorage> Storage;
			CComPtr<IStorage> Storage;

			//_com_ptr_t<IViewObject> View;
			CComPtr<IViewObject> View;

			// _com_ptr_t<IDataObject> Data;
			CComPtr<IDataObject> Data;

			//_com_ptr_t<IOleInPlaceActiveObject> Pao;
			CComPtr<IOleInPlaceActiveObject> Pao;

			//_com_ptr_t<IServiceProvider> CustomServiceProvider;
			CComPtr<IServiceProvider> CustomServiceProvider;

			AX_CONNECTSTRUCT* tcs;
			bool AddMenu;
			DWORD AdviseToken;
			DWORD DAdviseToken[100];
			HWND CommandCallbackWindow;
			HWND DispatchNotificationWindow;
			void(__stdcall* DispatchNotificationFunction)(class AXDISPATCHNOTIFICATION*);
			UINT DispatchNotificationMessage;
			void SetProcessCmd(HRESULT(*y)(char*, LPARAM)) { pcmd = y; }

			HWND hPar = 0;
			UINT hClosingMessage = 0;


		private:

			CLSID clsid;
			HRESULT(*pcmd)(char*, LPARAM);


		};


		class AXDISPATCHNOTIFICATION
		{
		public:

			class AX* ax;
			DISPID dispIdMember;
			CLSID riid;
			LCID lcid;
			WORD wFlags;
			DISPPARAMS FAR* pDispParams;
			VARIANT FAR* pVarResult;
			EXCEPINFO FAR* pExcepInfo;
			unsigned int FAR* puArgErr;

			/*
				AXDISPATCHNOTIFICATION()
					{
					ax = 0;
					dispIdMember = 0;
					memset(&riid,0,sizeof(riid));
					wFlags = 0;
					lcid = 0;
					pDispParams = 0;
					pVarResult = 0;
					pExcepInfo = 0;
					puArgErr = 0;
					}*/
		};


		// AXClientSite class
		// ------- Implement member functions
		inline AXClientSite::AXClientSite()
		{
			refNum = 0;
			CalledCanInPlace = 0;
			InPlace = 0;
		}

		inline AXClientSite :: ~AXClientSite()
		{
		}


		// IUnknown methods
		inline STDMETHODIMP AXClientSite::QueryInterface(REFIID iid, void** ppvObject)
		{
			*ppvObject = 0;
			if (iid == IID_IOleClientSite)
				* ppvObject = (IOleClientSite*)this;
			if (iid == IID_IUnknown)
				* ppvObject = this;
			if (iid == IID_IAdviseSink)
				* ppvObject = (IAdviseSink*)this;
			if (iid == IID_IDispatch)
				* ppvObject = (IDispatch*)this;
			if (iid == SpecialIIDForceOK)
				* ppvObject = (IDispatch*)this;
			if (iid == IID_IServiceProvider)
				* ppvObject = (IServiceProvider*)this;
			if (ExternalPlace == false)
			{
				if (iid == IID_IOleInPlaceSite)
					* ppvObject = (IOleInPlaceSite*)this;
				if (iid == IID_IOleInPlaceFrame)
					* ppvObject = (IOleInPlaceFrame*)this;
				if (iid == IID_IOleInPlaceUIWindow)
					* ppvObject = (IOleInPlaceUIWindow*)this;
			}

			//* Log Call
			if (*ppvObject)
			{
				this->AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		inline STDMETHODIMP_(ULONG)  AXClientSite::AddRef()
		{
			refNum++;
			return refNum;
		}

		inline STDMETHODIMP_(ULONG)  AXClientSite::Release()
		{
			refNum--;
			return refNum;
		}

		// IServiceManager 	
		inline STDMETHODIMP AXClientSite::QueryService(REFGUID guid, REFIID iid, void** ppvObject)
		{
			if (!ax->CustomServiceProvider)
				return E_NOINTERFACE;
			return ax->CustomServiceProvider->QueryService(guid, iid, ppvObject);
		}


		// IOleClientSite methods
		inline STDMETHODIMP AXClientSite::SaveObject()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::GetMoniker(DWORD dwA, DWORD dwW, IMoniker * *pm)
		{
			*pm = 0;
			return E_NOTIMPL;
		}

		inline STDMETHODIMP AXClientSite::GetContainer(IOleContainer * *pc)
		{
			*pc = 0;
			return E_FAIL;
		}

		inline STDMETHODIMP AXClientSite::ShowObject()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::OnShowWindow(BOOL f)
		{
			InvalidateRect(Window, 0, TRUE);
			InvalidateRect(Parent, 0, TRUE);
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::RequestNewObjectLayout()
		{
			return S_OK;
		}

		inline STDMETHODIMP_(void) AXClientSite::OnViewChange(DWORD dwAspect, LONG lIndex)
		{
		}

		inline STDMETHODIMP_(void) AXClientSite::OnRename(IMoniker * pmk)
		{
		}

		inline STDMETHODIMP_(void) AXClientSite::OnSave()
		{
		}

		inline STDMETHODIMP_(void) AXClientSite::OnClose()
		{
		}

		// IOleInPlaceSite methods
		inline STDMETHODIMP AXClientSite::GetWindow(HWND * p)
		{
			*p = Window;
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::ContextSensitiveHelp(BOOL)
		{
			return E_NOTIMPL;
		}


		inline STDMETHODIMP AXClientSite::CanInPlaceActivate()
		{
			if (InPlace)
			{
				CalledCanInPlace = true;
				return S_OK;
			}
			return S_FALSE;
		}

		inline STDMETHODIMP AXClientSite::OnInPlaceActivate()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::OnUIActivate()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::GetWindowContext(IOleInPlaceFrame * *ppFrame, IOleInPlaceUIWindow * *ppDoc, LPRECT r1, LPRECT r2, LPOLEINPLACEFRAMEINFO o)
		{
			*ppFrame = (IOleInPlaceFrame*)this;
			AddRef();

			*ppDoc = NULL;
			GetClientRect(Window, r1);
			GetClientRect(Window, r2);
			o->cb = sizeof(OLEINPLACEFRAMEINFO);
			o->fMDIApp = false;
			o->hwndFrame = Parent;
			o->haccel = 0;
			o->cAccelEntries = 0;

			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::Scroll(SIZE s)
		{
			return E_NOTIMPL;
		}

		inline STDMETHODIMP AXClientSite::OnUIDeactivate(int)
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::OnInPlaceDeactivate()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::DiscardUndoState()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::DeactivateAndUndo()
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::OnPosRectChange(LPCRECT)
		{
			return S_OK;
		}


		// IOleInPlaceFrame methods
		inline STDMETHODIMP AXClientSite::GetBorder(LPRECT l)
		{
			GetClientRect(Window, l);
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::RequestBorderSpace(LPCBORDERWIDTHS b)
		{
			//return S_OK;
			return E_NOTIMPL;
		}

		inline STDMETHODIMP AXClientSite::SetBorderSpace(LPCBORDERWIDTHS b)
		{
			return S_OK;
		}

		inline STDMETHODIMP AXClientSite::SetActiveObject(IOleInPlaceActiveObject * pV, LPCOLESTR s)
		{
			ax->Pao = pV;
			return S_OK;
		}


		inline STDMETHODIMP AXClientSite::SetStatusText(LPCOLESTR t)
		{
			return E_NOTIMPL;
		}

		inline STDMETHODIMP AXClientSite::EnableModeless(BOOL f)
		{
			return E_NOTIMPL;
		}

		inline STDMETHODIMP AXClientSite::TranslateAccelerator(LPMSG, WORD)
		{
			return E_NOTIMPL;
		}

		// IDispatch Methods
		inline HRESULT _stdcall AXClientSite::GetTypeInfoCount(
			unsigned int* pctinfo) {
			return E_NOTIMPL;
		}

		inline HRESULT _stdcall AXClientSite::GetTypeInfo(
			unsigned int iTInfo,
			LCID lcid,
			ITypeInfo FAR * FAR * ppTInfo) {
			return E_NOTIMPL;
		}

		inline HRESULT _stdcall AXClientSite::GetIDsOfNames(
			REFIID riid,
			OLECHAR FAR * FAR*,
			unsigned int cNames,
			LCID lcid,
			DISPID FAR*) {
			return E_NOTIMPL;
		}


		// Other Methods
		inline void AX::Init(char* cls)
		{
			pcmd = 0;
			wchar_t x[1000] = { 0 };
			MultiByteToWideChar(CP_ACP, 0, cls, -1, x, 1000);
			CLSIDFromString(x, &clsid);
			iid = (IID*)& IID_IOleObject;
			OleObject = 0;
			Storage = 0;
			View = 0;
			Data = 0;
			Pao = 0;
			AdviseToken = 0;
			CustomServiceProvider = 0;
			memset(DAdviseToken, 0, sizeof(DAdviseToken));
			Site.ax = this;
			DispatchNotificationFunction = 0;
			DispatchNotificationWindow = 0;
			DispatchNotificationMessage = 0;
		}

		inline AX::AX(char* cls)
		{
			Init(cls);
		}



		inline void AX::Clean()
		{
			if (Site.InPlace == true)
			{
				Site.InPlace = false;
				CComPtr<IOleInPlaceObject> iib = 0;
				if (OleObject)
					OleObject->QueryInterface(IID_IOleInPlaceObject, (void**)& iib);
				if (iib)
				{
					iib->UIDeactivate();
					iib->InPlaceDeactivate();
				}
			}

			if (AdviseToken && OleObject)
			{
				OleObject->Unadvise(AdviseToken);
				AdviseToken = 0;
			}
			if (Data)
			{
				for (int i = 0; i < 100; i++)
					if (DAdviseToken[i])
						Data->DUnadvise(DAdviseToken[i]);
				memset(DAdviseToken, 0, sizeof(DAdviseToken));
			}


			Pao = 0;
			Data = 0;
			View = 0;
			Storage = 0;
			OleObject = 0;
		}

		inline AX :: ~AX()
		{
			Clean();
		}

		inline CLSID AX::GetCLSID()
		{
			return clsid;
		}


		inline HRESULT _stdcall AXClientSite::InsertMenus(HMENU h, LPOLEMENUGROUPWIDTHS x)
		{
			/*      AX * t = (AX*)ax;
			if (t->AddMenu)
			{
			x->width[0] = 0;
			x->width[2] = 0;
			x->width[4] = 0;
			//InsertMenu(h,0,MF_BYPOSITION | MF_POPUP,(int)Menu,"test");
			return S_OK;
			}
			*/
			return E_NOTIMPL;
		}

		inline HRESULT _stdcall AXClientSite::SetMenu(HMENU h, HOLEMENU hO, HWND hw)
		{
			//	AX * t = (AX*)ax;
			/*      if (t->AddMenu)
			{
			if (!h && !hO)
			{
			//::SetMenu(Window,Menu);
			//DrawMenuBar(Window);
			::SetMenu(Parent,Menu);
			DrawMenuBar(Parent);
			return S_OK;
			}

			//::SetMenu(Window,h);
			//DrawMenuBar(Window);

			//HMENU hm = GetMenu(Parent);
			//AppendMenu(hm,MF_POPUP | MF_MENUBREAK,(int)h,0);
			//::SetMenu(Parent,hm);
			::SetMenu(Parent,h);
			DrawMenuBar(Parent);

			//hOleWindow = hw;
			//OleSetMenuDescriptor(hO,Window,hw,0,0);
			OleSetMenuDescriptor(hO,Parent,hw,0,0);

			return S_OK;
			}
			*/
			return E_NOTIMPL;
		}

		inline HRESULT _stdcall AXClientSite::RemoveMenus(HMENU h)
		{
#ifdef WINCE
			return E_NOTIMPL;
#else
			AX* t = (AX*)ax;
			if (t->AddMenu)
			{
				if (!h)
					return S_OK;

				int c = GetMenuItemCount(h);
				for (int i = c; i >= 0; i--)
				{
					HMENU hh = GetSubMenu(h, i);
					if (hh == Menu)
						RemoveMenu(h, i, MF_BYPOSITION);
				}
				if (h == Menu)
					DestroyMenu(h);

				//DrawMenuBar(Window);
				DrawMenuBar(Parent);
				return S_OK;
			}
			return E_NOTIMPL;
#endif
		}

		//extern HRESULT ProcessCmd(char*);
		inline HRESULT _stdcall AXClientSite::Invoke(
			DISPID dispIdMember,
			REFIID riid,
			LCID lcid,
			WORD wFlags,
			DISPPARAMS FAR * pDispParams,
			VARIANT FAR * pVarResult,
			EXCEPINFO FAR * pExcepInfo,
			unsigned int FAR * puArgErr)
		{
			AXDISPATCHNOTIFICATION axd = { ax,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr };
			if (ax->DispatchNotificationWindow)
			{
				SendMessage(ax->DispatchNotificationWindow, ax->DispatchNotificationMessage, 0, (LPARAM)& axd);
			}
			if (ax->DispatchNotificationFunction)
			{
				ax->DispatchNotificationFunction(&axd);
				return S_OK;
			}

			if (ax->pcmd == 0)
				return S_OK;

			// Check for DWebBrowserEvent2 :: BeforeNavigate2
			// dispid = 0xfa
			char zv[1000] = { 0 };

			if (dispIdMember == 0xfa && pDispParams->cArgs == 7)
			{
				for (unsigned int i = 0; i < pDispParams->cArgs; i++)
				{
					if (pDispParams->rgvarg[i].vt == (VT_VARIANT | VT_BYREF))
					{
						VARIANT* xv = pDispParams->rgvarg[i].pvarVal;
						if (xv->vt == VT_BSTR)
							WideCharToMultiByte(0, 0, xv->bstrVal, -1, zv, 1000, 0, 0);
						if (strncmp(zv, "app:", 4) == 0)
						{
							for (unsigned int x = 0; x < pDispParams->cArgs; x++)
							{
								if (pDispParams->rgvarg[x].vt == (VT_BOOL | VT_BYREF))
								{
									VARIANT_BOOL* y = pDispParams->rgvarg[x].pboolVal;
									*y = VARIANT_TRUE;
									break;
								}
							}
							return ax->pcmd(zv, (LPARAM)ax);
						}
					}
					if (pDispParams->rgvarg[i].vt == VT_BSTR)
					{
						VARIANT* xv = &pDispParams->rgvarg[i];
						if (xv->vt == VT_BSTR)
							WideCharToMultiByte(0, 0, xv->bstrVal, -1, zv, 1000, 0, 0);
						if (strncmp(zv, "app:", 4) == 0)
						{
							for (unsigned int x = 0; x < pDispParams->cArgs; x++)
							{
								if (pDispParams->rgvarg[x].vt == (VT_BOOL | VT_BYREF))
								{
									VARIANT_BOOL* y = pDispParams->rgvarg[x].pboolVal;
									*y = VARIANT_TRUE;
									break;
								}
							}
							return ax->pcmd(zv, (LPARAM)ax);
						}
					}
				}
				return S_OK;
			}

			return S_OK;
		}


		inline void _stdcall AXClientSite::OnDataChange(FORMATETC * pFormatEtc, STGMEDIUM * pStgmed)
		{
			// Notify our app that a change is being requested
			return;
		}



		// Window Procedure for AX control
		inline LRESULT CALLBACK AXWndProc(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
		{
			if (mm == WM_CLOSE)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				if (!ax->hPar)
					return 0;
				auto st = GetWindowLong(hh, GWL_STYLE);
				if (st & WS_CHILD)
					return DefWindowProc(hh, mm, ww, ll);
				return SendMessage(ax->hPar, ax->hClosingMessage, (WPARAM)hh, 0);
			}
			if (mm == WM_CREATE || mm == AX_RECREATE)
			{
				AX* ax;
				char tit[1000] = { 0 };
				HRESULT hr;
				wchar_t wtit[1000] = { 0 };
				GetWindowTextW(hh, wtit, 1000);
				WideCharToMultiByte(CP_ACP, 0, wtit, -1, tit, 1000, 0, 0);
				if (mm == WM_CREATE)
				{
					if (tit[0] == '}')
						return 0; // No Creation at the momemt
				}
				if (mm == AX_RECREATE) // ll is the IUnknown
				{
					tit[0] = '{';
				}


				ax = new AX(tit);
				SetWindowLongPtr(hh, GWLP_USERDATA, (LONG_PTR)ax);
				ax->Site.Window = hh;
				ax->Site.ax = ax;
				ax->Site.Parent = GetParent(hh);

				hr = StgCreateDocfile(0, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE, 0, &ax->Storage);
				ax->Site.Window = hh;

				REFIID rid = *ax->iid;
				if (mm == WM_CREATE)
				{
					CLSID cid = ax->GetCLSID();
					hr = OleCreate(cid, rid, OLERENDER_DRAW, 0, &ax->Site, ax->Storage, (void**)& ax->OleObject);
					if (FAILED(hr))
					{
						hr = OleCreate(cid, rid, OLERENDER_NONE, 0, &ax->Site, ax->Storage, (void**)& ax->OleObject);
					}
				}
				else
					if (mm == AX_RECREATE)
					{
						IUnknown* u = (IUnknown*)ll;
						if (u)
							u->QueryInterface(rid, (void**)& ax->OleObject);
					}

				if (!ax->OleObject)
				{
					delete ax;
					SetWindowLongPtr(hh, GWLP_USERDATA, 0);
					return -1;
				}
				ax->OleObject->SetClientSite(&ax->Site);

				hr = OleSetContainedObject(ax->OleObject, TRUE);
				hr = ax->OleObject->Advise(&ax->Site, &ax->AdviseToken);
				hr = ax->OleObject->QueryInterface(IID_IViewObject, (void**)& ax->View);
				hr = ax->OleObject->QueryInterface(IID_IDataObject, (void**)& ax->Data);
				if (ax->View)
					hr = ax->View->SetAdvise(DVASPECT_CONTENT, 0, &ax->Site);



				return 0;
			}



			if (mm == WM_DESTROY)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				ax->Clean();
				return true;
			}


			if (mm == WM_COMMAND)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				return SendMessage(ax->CommandCallbackWindow, mm, ww, ll);
			}

			/*
			if (mm == AX_SETSERVICEPROVIDER)
			{
			AX* ax = (AX*)GetWindowLongPtr(hh,GWLP_USERDATA);
			if (!ax)
			return 0;
			ax->CustomServiceProvider = (IServiceProvider*)ll;
			return 1;
			}
			*/

			if (mm == AX_CONNECTOBJECT)
			{
				// ww = (IID*)  Interface ID to which we request connection
				// ll = (void*) Pointer to a AX_CONNECTSTRUCT to fill in
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				int* wx = (int*)ww;
				AX_CONNECTSTRUCT* tcs = (AX_CONNECTSTRUCT*)ll;

				if (tcs->Advisor == 0)
				{
					tcs->Advisor = (IUnknown*)& ax->Site;
					ax->Site.SpecialIIDForceOK = tcs->SpecialIIDForceOK;
				}
				tcs->id = AXConnectObject((IUnknown*)ax->OleObject, (REFIID)* wx, tcs->Advisor, &tcs->icpc, &tcs->icp);
				ax->tcs = new AX_CONNECTSTRUCT;
				memcpy(ax->tcs, tcs, sizeof(AX_CONNECTSTRUCT));
				return 0;
			}

			if (mm == AX_DISCONNECTOBJECT)
			{
				// Use of ConnectObject
				// ww = (IID*)  Interface ID to which we request connection
				//	char* p = (char*)ww;
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				AX_CONNECTSTRUCT* tcs = ax->tcs;
				AXDisconnectObject(tcs->icpc, tcs->icp, tcs->id);
				delete ax->tcs;
				ax->tcs = 0;
				return 0;
			}


			if (mm == AX_SETCOMMANDCALLBACK)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				ax->CommandCallbackWindow = (HWND)ww;
				return 1;
			}

			if (mm == AX_SETDISPATCHNOTIFICATION)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				// ww = Window
				// ll = Message
				ax->DispatchNotificationWindow = (HWND)ww;
				ax->DispatchNotificationMessage = (UINT)ll;
				return 1;
			}

			if (mm == AX_SETDISPATCHNOTIFICATIONFUNC)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				ax->DispatchNotificationFunction = (void(__stdcall*)(AXDISPATCHNOTIFICATION*))ww;
				return 1;
			}

			if (mm == AX_SETDATAADVISE)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;

				// 1.Enum available FORMATETC structures
				// 2.Set Data Advise specified to index ww
				if (!ax->Data)
					return 0;

				CComPtr<IEnumFORMATETC> ief = 0;
				int i = 0;

				FORMATETC fe;
				ax->Data->EnumFormatEtc((DWORD)ll, &ief);
				if (!ief)
					return 0;
				for (;;)
				{
					HRESULT hr = ief->Next(1, &fe, 0);
					if (hr != S_OK)
						break;
					if (ww == (WPARAM)i)
						break;
					i++;
				}
				if (ww == -1)
					return i;

				if (ax->Data)
					ax->Data->DAdvise(&fe, 0, &ax->Site, &ax->DAdviseToken[ww]);

				return true;
			}

			if (mm == AX_GETAXINTERFACE)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				return (LONG_PTR)ax;
			}

			if (mm == AX_QUERYINTERFACE)
			{
				char* p = (char*)ww;
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				return ax->OleObject->QueryInterface((REFIID)* p, (void**)ll);
			}

			if (mm == WM_LBUTTONDBLCLK)
			{
				PostMessage(hh, AX_INPLACE, 1, 0);
				return 0;
			}

			if (mm == AX_SETPARENT)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				ax->hPar = (HWND)ww;
				ax->hClosingMessage = (UINT)ll;
				return 0;
			}


			if (mm == AX_INPLACE)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				if (!ax->OleObject)
					return 0;
				RECT rect;
				HRESULT hr;
				::GetClientRect(hh, &rect);

				if (ax->Site.InPlace == false && ww == 1) // Activate In Place
				{
					ax->Site.InPlace = true;
					ax->Site.ExternalPlace = (int)ll;
					hr = ax->OleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, &ax->Site, 0, hh, &rect);
					InvalidateRect(hh, 0, true);
					return 1;
				}

				if (ax->Site.InPlace == true && ww == 0) // Deactivate
				{
					ax->Site.InPlace = false;

					CComPtr<IOleInPlaceObject> iib;
					ax->OleObject->QueryInterface(IID_IOleInPlaceObject, (void**)& iib);
					if (iib)
					{
						iib->UIDeactivate();
						iib->InPlaceDeactivate();
						InvalidateRect(hh, 0, true);
						return 1;
					}
				}
				return 0;
			}

			if (mm == WM_SIZE)
			{
				AX* ax = (AX*)GetWindowLongPtr(hh, GWLP_USERDATA);
				if (!ax)
					return 0;
				if (!ax->OleObject)
					return 0;

				// Send Notification to parent
				NMHDR nh = { 0 };
				nh.code = AXN_SIZE;
				nh.hwndFrom = hh;
				nh.idFrom = GetWindowLong(hh, GWL_ID);
				SendMessage(ax->Site.Parent, WM_NOTIFY, 0, (LPARAM)& nh);
				DefWindowProc(hh, mm, ww, ll);
				/*
				if (ax->Site.InPlace == true)
				{
				SendMessage(hh,AX_INPLACE,0,0);
				InvalidateRect(hh,0,true);
				SendMessage(hh,AX_INPLACE,1,0);
				}
				*/

				CComPtr<IOleInPlaceObject> pl;
				ax->OleObject->QueryInterface(IID_IOleInPlaceObject, (void**)& pl);
				if (!pl)
					return 0;
				RECT r;
				GetClientRect(ax->Site.Window, &r);
				pl->SetObjectRects(&r, &r);
				return 0;
			}

			return DefWindowProc(hh, mm, ww, ll);
		}



		inline int AXConnectObject(IUnknown * Container, REFIID riid, IUnknown * Advisor, IConnectionPointContainer * *picpc, IConnectionPoint * *picp)
		{
			HRESULT hr = 0;
			unsigned long tid = 0;
			IConnectionPointContainer* icpc = 0;
			IConnectionPoint* icp = 0;
			*picpc = 0;
			*picp = 0;

			Container->QueryInterface(IID_IConnectionPointContainer, (void**)& icpc);
			if (icpc)
			{
				*picpc = icpc;
				icpc->FindConnectionPoint(riid, &icp);
				if (icp)
				{
					*picp = icp;
					hr = icp->Advise(Advisor, &tid);
				}
			}
			return tid;
		}

		inline void AXDisconnectObject(IConnectionPointContainer * icpc, IConnectionPoint * icp, unsigned int Cookie)
		{
			//unsigned long hr  = 0;
			icp->Unadvise(Cookie);
			icp->Release();
			icpc->Release();
		}



		// Registration function
		inline ATOM AXRegister()
		{
#ifdef WINCE
			WNDCLASS wC = { 0 };
#else
			WNDCLASSEXW wC = { 0 };
			wC.cbSize = sizeof(wC);
#endif

			wC.style = CS_GLOBALCLASS | CS_DBLCLKS;
			wC.lpfnWndProc = AXWndProc;
			wC.cbWndExtra = sizeof(void*);
			wC.hInstance = GetModuleHandle(0);
			wC.lpszClassName = L"AX";
#ifdef WINCE
			return RegisterClassW(&wC);
#else
			return RegisterClassExW(&wC);
#endif
		}





	}





	// JSONXX
	// -*- mode: c++; c-basic-offset: 4; -*-

	// Author: Hong Jiang <hong@hjiang.net>
	// Contributors:
	//   Sean Middleditch <sean@middleditch.us>
	//   rlyeh <https://github.com/r-lyeh>

#pragma warning(disable:4127)

// jsonxx versioning: major.minor-extra where
// major = { number }
// minor = { number }
// extra = { 'a':alpha, 'b':beta, 'rc': release candidate, 'r': release, 's':stable }
#define JSONXX_MAJOR    "0"
#define JSONXX_MINOR    "22"
#define JSONXX_EXTRA    "a"
#define JSONXX_VERSION  JSONXX_MAJOR "." JSONXX_MINOR "-" JSONXX_EXTRA
#define JSONXX_XML_TAG  "<!-- generated by jsonxx " JSONXX_VERSION " -->"

#if __cplusplus > 199711L
#define JSONXX_COMPILER_HAS_CXX11 1
#elif defined(_MSC_VER) && _MSC_VER > 1700
#define JSONXX_COMPILER_HAS_CXX11 1
#else
#define JSONXX_COMPILER_HAS_CXX11 0
#endif

#define JSONXX_ASSERT(...) do { if( jsonxx::Assertions ) \
  jsonxx::assertion(__FILE__,__LINE__,#__VA_ARGS__,bool(__VA_ARGS__)); } while(0)

	namespace jsonxx {

		// Settings
		enum Settings {
			// constants
			Enabled = true,
			Disabled = false,
			Permissive = true,
			Strict = false,
			// values
			Parser = Permissive,  // permissive or strict parsing
			UnquotedKeys = Disabled, // support of unquoted keys
			Assertions = Enabled  // enabled or disabled assertions (these asserts work both in DEBUG and RELEASE builds)
		};

		// Constants for .write() and .xml() methods
		enum Format {
			JSON = 0,     // JSON output
			JSONx = 1,     // XML output, JSONx  format. see http://goo.gl/I3cxs
			JXML = 2,     // XML output, JXML   format. see https://github.com/r-lyeh/JXML
			JXMLex = 3,     // XML output, JXMLex format. see https://github.com/r-lyeh/JXMLex
			TaggedXML = 4      // XML output, tagged XML format. see https://github.com/hjiang/jsonxx/issues/12
		};

		// Types
		typedef long double Number;
		typedef bool Boolean;
		typedef std::string String;
		struct Null {};
		class Value;
		class Object;
		class Array;

		// Identity meta-function
		template <typename T>
		struct identity {
			typedef T type;
		};

		// Tools
		bool validate(const std::string& input);
		bool validate(std::istream& input);
		std::string reformat(const std::string& input);
		std::string reformat(std::istream& input);
		std::string xml(const std::string& input, unsigned format = JSONx);
		std::string xml(std::istream& input, unsigned format = JSONx);

		// Detail
		void assertion(const char* file, int line, const char* expression, bool result);

		// A JSON Object
		class Object {
		public:
			Object();
			~Object();

			template <typename T>
			bool has(const std::string& key) const;

			// Always call has<>() first. If the key doesn't exist, consider
			// the behavior undefined.
			template <typename T>
			T& get(const std::string& key);
			template <typename T>
			const T& get(const std::string& key) const;

			template <typename T>
			const T& get(const std::string& key, const typename identity<T>::type& default_value) const;

			size_t size() const;
			bool empty() const;

			const std::map<std::string, Value*>& kv_map() const;
			std::string json() const;
			std::string xml(unsigned format = JSONx, const std::string& header = std::string(), const std::string& attrib = std::string()) const;
			std::string write(unsigned format) const;

			void reset();
			bool parse(std::istream& input);
			bool parse(const std::string& input);
			typedef std::map<std::string, Value*> container;
			void import(const Object& other);
			void import(const std::string& key, const Value& value);
			Object& operator<<(const Value& value);
			Object& operator<<(const Object& value);
			Object& operator=(const Object& value);
			Object(const Object& other);
			Object(const std::string& key, const Value& value);
			template<size_t N>
			Object(const char(&key)[N], const Value& value) {
				import(key, value);
			}
			template<typename T>
			Object& operator<<(const T& value);

		protected:
			static bool parse(std::istream& input, Object& object);
			container value_map_;
			std::string odd;
		};

		class Array {
		public:
			Array();
			~Array();

			size_t size() const;
			bool empty() const;

			template <typename T>
			bool has(unsigned int i) const;

			template <typename T>
			T& get(unsigned int i);
			template <typename T>
			const T& get(unsigned int i) const;

			template <typename T>
			const T& get(unsigned int i, const typename identity<T>::type& default_value) const;

			const std::vector<Value*>& values() const {
				return values_;
			}
			std::string json() const;
			std::string xml(unsigned format = JSONx, const std::string& header = std::string(), const std::string& attrib = std::string()) const;

			std::string write(unsigned format) const { return format == JSON ? json() : xml(format); }
			void reset();
			bool parse(std::istream& input);
			bool parse(const std::string& input);
			typedef std::vector<Value*> container;
			void import(const Array& other);
			void import(const Value& value);
			Array& operator<<(const Array& other);
			Array& operator<<(const Value& value);
			Array& operator=(const Array& other);
			Array& operator=(const Value& value);
			Array(const Array& other);
			Array(const Value& value);
		protected:
			static bool parse(std::istream& input, Array& array);
			container values_;
		};

		// A value could be a number, an array, a string, an object, a
		// boolean, or null
		class Value {
		public:

			Value();
			~Value() { reset(); }
			void reset();

			template<typename T>
			void import(const T&) {
				reset();
				type_ = INVALID_;
				// debug
				// std::cout << "[WARN] No support for " << typeid(t).name() << std::endl;
			}
			void import(const bool& b) {
				reset();
				type_ = BOOL_;
				bool_value_ = b;
			}
#define $number(TYPE) \
  void import( const TYPE &n ) { \
    reset(); \
    type_ = NUMBER_; \
    number_value_ = static_cast<long double>(n); \
  }
			$number(char)
				$number(int)
				$number(long)
				$number(long long)
				$number(unsigned char)
				$number(unsigned int)
				$number(unsigned long)
				$number(unsigned long long)
				$number(float)
				$number(double)
				$number(long double)
#undef $number
#if JSONXX_COMPILER_HAS_CXX11 > 0
				void import(const std::nullptr_t&) {
				reset();
				type_ = NULL_;
			}
#endif
			void import(const Null&) {
				reset();
				type_ = NULL_;
			}
			void import(const String& s) {
				reset();
				type_ = STRING_;
				*(string_value_ = new String()) = s;
			}
			void import(const Array & a) {
				reset();
				type_ = ARRAY_;
				*(array_value_ = new Array()) = a;
			}
			void import(const Object & o) {
				reset();
				type_ = OBJECT_;
				*(object_value_ = new Object()) = o;
			}
			void import(const Value & other) {
				if (this != &other)
					switch (other.type_) {
					case NULL_:
						import(Null());
						break;
					case BOOL_:
						import(other.bool_value_);
						break;
					case NUMBER_:
						import(other.number_value_);
						break;
					case STRING_:
						import(*other.string_value_);
						break;
					case ARRAY_:
						import(*other.array_value_);
						break;
					case OBJECT_:
						import(*other.object_value_);
						break;
					case INVALID_:
						type_ = INVALID_;
						break;
					default:
						JSONXX_ASSERT(!"not implemented");
					}
			}
			template<typename T>
			Value& operator <<(const T & t) {
				import(t);
				return *this;
			}
			template<typename T>
			Value& operator =(const T & t) {
				reset();
				import(t);
				return *this;
			}
			Value(const Value & other);
			template<typename T>
			Value(const T & t) : type_(INVALID_) { import(t); }
			template<size_t N>
			Value(const char(&t)[N]) : type_(INVALID_) { import(std::string(t)); }

			bool parse(std::istream & input);
			bool parse(const std::string & input);

			template<typename T>
			bool is() const;
			template<typename T>
			T& get();
			template<typename T>
			const T& get() const;

			bool empty() const;

		public:
			enum {
				NUMBER_,
				STRING_,
				BOOL_,
				NULL_,
				ARRAY_,
				OBJECT_,
				INVALID_
			} type_;
			union {
				Number number_value_;
				String* string_value_;
				Boolean bool_value_;
				Array* array_value_;
				Object* object_value_;
			};

		protected:
			static bool parse(std::istream & input, Value & value);
		};

		template <typename T>
		bool Array::has(unsigned int i) const {
			if (i >= size()) {
				return false;
			}
			else {
				Value* v = values_.at(i);
				return v->is<T>();
			}
		}

		template <typename T>
		T& Array::get(unsigned int i) {
			JSONXX_ASSERT(i < size());
			Value* v = values_.at(i);
			return v->get<T>();
		}

		template <typename T>
		const T& Array::get(unsigned int i) const {
			JSONXX_ASSERT(i < size());
			const Value* v = values_.at(i);
			return v->get<T>();
		}

		template <typename T>
		const T& Array::get(unsigned int i, const typename identity<T>::type & default_value) const {
			if (has<T>(i)) {
				const Value* v = values_.at(i);
				return v->get<T>();
			}
			else {
				return default_value;
			}
		}

		template <typename T>
		bool Object::has(const std::string & key) const {
			container::const_iterator it(value_map_.find(key));
			return it != value_map_.end() && it->second->is<T>();
		}

		template <typename T>
		T& Object::get(const std::string & key) {
			JSONXX_ASSERT(has<T>(key));
			return value_map_.find(key)->second->get<T>();
		}

		template <typename T>
		const T& Object::get(const std::string & key) const {
			JSONXX_ASSERT(has<T>(key));
			return value_map_.find(key)->second->get<T>();
		}

		template <typename T>
		const T& Object::get(const std::string & key, const typename identity<T>::type & default_value) const {
			if (has<T>(key)) {
				return value_map_.find(key)->second->get<T>();
			}
			else {
				return default_value;
			}
		}

		template<>
		inline bool Value::is<Value>() const {
			return true;
		}

		template<>
		inline bool Value::is<Null>() const {
			return type_ == NULL_;
		}

		template<>
		inline bool Value::is<Boolean>() const {
			return type_ == BOOL_;
		}

		template<>
		inline bool Value::is<String>() const {
			return type_ == STRING_;
		}

		template<>
		inline bool Value::is<Number>() const {
			return type_ == NUMBER_;
		}

		template<>
		inline bool Value::is<Array>() const {
			return type_ == ARRAY_;
		}

		template<>
		inline bool Value::is<Object>() const {
			return type_ == OBJECT_;
		}

		template<>
		inline Value& Value::get<Value>() {
			return *this;
		}

		template<>
		inline const Value& Value::get<Value>() const {
			return *this;
		}

		template<>
		inline bool& Value::get<Boolean>() {
			JSONXX_ASSERT(is<Boolean>());
			return bool_value_;
		}

		template<>
		inline std::string& Value::get<String>() {
			JSONXX_ASSERT(is<String>());
			return *string_value_;
		}

		template<>
		inline Number& Value::get<Number>() {
			JSONXX_ASSERT(is<Number>());
			return number_value_;
		}

		template<>
		inline Array& Value::get<Array>() {
			JSONXX_ASSERT(is<Array>());
			return *array_value_;
		}

		template<>
		inline Object& Value::get<Object>() {
			JSONXX_ASSERT(is<Object>());
			return *object_value_;
		}

		template<>
		inline const Boolean& Value::get<Boolean>() const {
			JSONXX_ASSERT(is<Boolean>());
			return bool_value_;
		}

		template<>
		inline const String& Value::get<String>() const {
			JSONXX_ASSERT(is<String>());
			return *string_value_;
		}

		template<>
		inline const Number& Value::get<Number>() const {
			JSONXX_ASSERT(is<Number>());
			return number_value_;
		}

		template<>
		inline const Array& Value::get<Array>() const {
			JSONXX_ASSERT(is<Array>());
			return *array_value_;
		}

		template<>
		inline const Object& Value::get<Object>() const {
			JSONXX_ASSERT(is<Object>());
			return *object_value_;
		}

		template<typename T>
		inline Object& Object::operator<<(const T & value) {
			return *this << Value(value), * this;
		}

	}  // namespace jsonxx

	std::ostream& operator<<(std::ostream & stream, const jsonxx::Value & v);
	std::ostream& operator<<(std::ostream & stream, const jsonxx::Object & v);
	std::ostream& operator<<(std::ostream & stream, const jsonxx::Array & v);



	// Implementation


	// -*- mode: c++; c-basic-offset: 4; -*-

	// Author: Hong Jiang <hong@hjiang.net>
	// Contributors:
	//   Sean Middleditch <sean@middleditch.us>
	//   rlyeh <https://github.com/r-lyeh>


	// Snippet that creates an assertion function that works both in DEBUG & RELEASE mode.
	// JSONXX_ASSERT(...) macro will redirect to this. assert() macro is kept untouched.
#if defined(NDEBUG) || defined(_NDEBUG)
#   define JSONXX_REENABLE_NDEBUG
#   undef  NDEBUG
#   undef _NDEBUG
#endif
	inline void jsonxx::assertion(const char* file, int line, const char* expression, bool result) {
		if (!result) {
			fprintf(stderr, "[JSONXX] expression '%s' failed at %s:%d -> ", expression, file, line);
//			assert(0);
		}
	}
#if defined(JSONXX_REENABLE_NDEBUG)
#   define  NDEBUG
#   define _NDEBUG
#endif

	namespace jsonxx {

		//static_assert( sizeof(unsigned long long) < sizeof(long double), "'long double' cannot hold 64bit values in this compiler :(");

		bool match(const char* pattern, std::istream& input);
		bool parse_array(std::istream& input, Array& array);
		bool parse_bool(std::istream& input, Boolean& value);
		bool parse_comment(std::istream& input);
		bool parse_null(std::istream& input);
		bool parse_number(std::istream& input, Number& value);
		bool parse_object(std::istream& input, Object& object);
		bool parse_string(std::istream& input, String& value);
		bool parse_identifier(std::istream& input, String& value);
		bool parse_value(std::istream& input, Value& value);

		// Try to consume characters from the input stream and match the
		// pattern string.
		inline bool match(const char* pattern, std::istream& input) {
			input >> std::ws;
			const char* cur(pattern);
			char ch(0);
			while (input && !input.eof() && *cur != 0) {
				input.get(ch);
				if (ch != *cur) {
					input.putback(ch);
					if (parse_comment(input))
						continue;
					while (cur > pattern) {
						cur--;
						input.putback(*cur);
					}
					return false;
				}
				else {
					cur++;
				}
			}
			return *cur == 0;
		}

		inline bool parse_string(std::istream& input, String& value) {
			char ch = '\0', delimiter = '"';
			if (!match("\"", input)) {
				if (Parser == Strict) {
					return false;
				}
				delimiter = '\'';
				if (input.peek() != delimiter) {
					return false;
				}
				input.get(ch);
			}
			while (!input.eof() && input.good()) {
				input.get(ch);
				if (ch == delimiter) {
					break;
				}
				if (ch == '\\') {
					input.get(ch);
					switch (ch) {
					case '\\':
					case '/':
						value.push_back(ch);
						break;
					case 'b':
						value.push_back('\b');
						break;
					case 'f':
						value.push_back('\f');
						break;
					case 'n':
						value.push_back('\n');
						break;
					case 'r':
						value.push_back('\r');
						break;
					case 't':
						value.push_back('\t');
						break;
					case 'u': {
						int i;
						std::stringstream ss;
						for (i = 0; (!input.eof() && input.good()) && i < 4; ++i) {
							input.get(ch);
							ss << std::hex << ch;
						}
						if (input.good() && (ss >> i))
							value.push_back((char)i);
					}
							  break;
					default:
						if (ch != delimiter) {
							value.push_back('\\');
							value.push_back(ch);
						}
						else value.push_back(ch);
						break;
					}
				}
				else {
					value.push_back(ch);
				}
			}
			if (input && ch == delimiter) {
				return true;
			}
			else {
				return false;
			}
		}

		inline bool parse_identifier(std::istream & input, String & value) {
			input >> std::ws;

			char ch = '\0', delimiter = ':';
			bool first = true;

			while (!input.eof() && input.good()) {
				input.get(ch);

				if (ch == delimiter) {
					input.unget();
					break;
				}

				if (first) {
					if ((ch != '_' && ch != '$') &&
						(ch < 'a' || ch > 'z') &&
						(ch < 'A' || ch > 'Z')) {
						return false;
					}
					first = false;
				}
				if (ch == '_' || ch == '$' ||
					(ch >= 'a' && ch <= 'z') ||
					(ch >= 'A' && ch <= 'Z') ||
					(ch >= '0' && ch <= '9')) {
					value.push_back(ch);
				}
				else if (ch == '\t' || ch == ' ') {
					input >> std::ws;
				}
			}
			if (input && ch == delimiter) {
				return true;
			}
			else {
				return false;
			}
		}

		inline bool parse_number(std::istream & input, Number & value) {
			input >> std::ws;
			std::streampos rollback = input.tellg();
			input >> value;
			if (input.fail()) {
				input.clear();
				input.seekg(rollback);
				return false;
			}
			return true;
		}

		inline bool parse_bool(std::istream & input, Boolean & value) {
			if (match("true", input)) {
				value = true;
				return true;
			}
			if (match("false", input)) {
				value = false;
				return true;
			}
			return false;
		}

		inline bool parse_null(std::istream & input) {
			if (match("null", input)) {
				return true;
			}
			if (Parser == Strict) {
				return false;
			}
			return (input.peek() == ',');
		}

		inline bool parse_array(std::istream & input, Array & array) {
			return array.parse(input);
		}

		inline bool parse_object(std::istream & input, Object & object) {
			return object.parse(input);
		}

		inline bool parse_comment(std::istream & input) {
			if (Parser == Permissive)
				if (!input.eof() && input.peek() == '/')
				{
					char ch0(0);
					input.get(ch0);

					if (!input.eof())
					{
						char ch1(0);
						input.get(ch1);

						if (ch0 == '/' && ch1 == '/')
						{
							// trim chars till \r or \n
							for (char ch(0); !input.eof() && (input.peek() != '\r' && input.peek() != '\n'); )
								input.get(ch);

							// consume spaces, tabs, \r or \n, in case no eof is found
							if (!input.eof())
								input >> std::ws;
							return true;
						}

						input.unget();
						input.clear();
					}

					input.unget();
					input.clear();
				}

			return false;
		}

		inline bool parse_value(std::istream & input, Value & value) {
			return value.parse(input);
		}


		inline Object::Object() : value_map_() {}

		inline Object::~Object() {
			reset();
		}

		inline bool Object::parse(std::istream & input, Object & object) {
			object.reset();

			if (!match("{", input)) {
				return false;
			}
			if (match("}", input)) {
				return true;
			}

			do {
				std::string key;
				if (UnquotedKeys == Enabled) {
					if (!parse_identifier(input, key)) {
						if (Parser == Permissive) {
							if (input.peek() == '}')
								break;
						}
						return false;
					}
				}
				else {
					if (!parse_string(input, key)) {
						if (Parser == Permissive) {
							if (input.peek() == '}')
								break;
						}
						return false;
					}
				}
				if (!match(":", input)) {
					return false;
				}
				Value* v = new Value();
				if (!parse_value(input, *v)) {
					delete v;
					break;
				}
				object.value_map_[key] = v;
			} while (match(",", input));


			if (!match("}", input)) {
				return false;
			}

			return true;
		}

		inline Value::Value() : type_(INVALID_) {}

		inline void Value::reset() {
			if (type_ == STRING_) {
				delete string_value_;
				string_value_ = 0;
			}
			else if (type_ == OBJECT_) {
				delete object_value_;
				object_value_ = 0;
			}
			else if (type_ == ARRAY_) {
				delete array_value_;
				array_value_ = 0;
			}
		}

		inline bool Value::parse(std::istream & input, Value & value) {
			value.reset();

			std::string string_value;
			if (parse_string(input, string_value)) {
				value.string_value_ = new std::string();
				value.string_value_->swap(string_value);
				value.type_ = STRING_;
				return true;
			}
			if (parse_number(input, value.number_value_)) {
				value.type_ = NUMBER_;
				return true;
			}

			if (parse_bool(input, value.bool_value_)) {
				value.type_ = BOOL_;
				return true;
			}
			if (parse_null(input)) {
				value.type_ = NULL_;
				return true;
			}
			if (input.peek() == '[') {
				value.array_value_ = new Array();
				if (parse_array(input, *value.array_value_)) {
					value.type_ = ARRAY_;
					return true;
				}
				delete value.array_value_;
			}
			value.object_value_ = new Object();
			if (parse_object(input, *value.object_value_)) {
				value.type_ = OBJECT_;
				return true;
			}
			delete value.object_value_;
			return false;
		}

		inline Array::Array() : values_() {}

		inline Array::~Array() {
			reset();
		}

		inline bool Array::parse(std::istream & input, Array & array) {
			array.reset();

			if (!match("[", input)) {
				return false;
			}
			if (match("]", input)) {
				return true;
			}

			do {
				Value* v = new Value();
				if (!parse_value(input, *v)) {
					delete v;
					break;
				}
				array.values_.push_back(v);
			} while (match(",", input));

			if (!match("]", input)) {
				return false;
			}
			return true;
		}

		inline static std::ostream& stream_string(std::ostream & stream,
			const std::string & string) {
			stream << '"';
			for (std::string::const_iterator i = string.begin(),
				e = string.end(); i != e; ++i) {
				switch (*i) {
				case '"':
					stream << "\\\"";
					break;
				case '\\':
					stream << "\\\\";
					break;
				case '/':
					stream << "\\/";
					break;
				case '\b':
					stream << "\\b";
					break;
				case '\f':
					stream << "\\f";
					break;
				case '\n':
					stream << "\\n";
					break;
				case '\r':
					stream << "\\r";
					break;
				case '\t':
					stream << "\\t";
					break;
				default:
					if (*i < 32) {
						stream << "\\u" << std::hex << std::setw(4) <<
							std::setfill('0') << static_cast<int>(*i) << std::dec <<
							std::setw(0);
					}
					else {
						stream << *i;
					}
				}
			}
			stream << '"';
			return stream;
		}

	}  // namespace jsonxx

	inline std::ostream& operator<<(std::ostream & stream, const jsonxx::Value & v) {
		using namespace jsonxx;
		if (v.is<Number>()) {
			return stream << v.get<Number>();
		}
		else if (v.is<String>()) {
			return stream_string(stream, v.get<std::string>());
		}
		else if (v.is<Boolean>()) {
			if (v.get<Boolean>()) {
				return stream << "true";
			}
			else {
				return stream << "false";
			}
		}
		else if (v.is<Null>()) {
			return stream << "null";
		}
		else if (v.is<Object>()) {
			return stream << v.get<Object>();
		}
		else if (v.is<Array>()) {
			return stream << v.get<Array>();
		}
		// Shouldn't reach here.
		return stream;
	}

	inline std::ostream& operator<<(std::ostream & stream, const jsonxx::Array & v) {
		stream << "[";
		jsonxx::Array::container::const_iterator
			it = v.values().begin(),
			end = v.values().end();
		while (it != end) {
			stream << *(*it);
			++it;
			if (it != end) {
				stream << ", ";
			}
		}
		return stream << "]";
	}

	inline std::ostream& operator<<(std::ostream & stream, const jsonxx::Object & v) {
		stream << "{";
		jsonxx::Object::container::const_iterator
			it = v.kv_map().begin(),
			end = v.kv_map().end();
		while (it != end) {
			jsonxx::stream_string(stream, it->first);
			stream << ": " << *(it->second);
			++it;
			if (it != end) {
				stream << ", ";
			}
		}
		return stream << "}";
	}


	namespace jsonxx {
		namespace {

			typedef unsigned char byte;

			//template<bool quote>
			inline std::string escape_string(const std::string& input, const bool quote = false) {
				static std::string map[256], * once = 0;
				if (!once) {
					// base
					for (int i = 0; i < 256; ++i) {
						map[i] = std::string() + char(i);
					}
					// non-printable
					for (int i = 0; i < 32; ++i) {
						std::stringstream str;
						str << "\\u" << std::hex << std::setw(4) << std::setfill('0') << i;
						map[i] = str.str();
					}
					// exceptions
					map[byte('"')] = "\\\"";
					map[byte('\\')] = "\\\\";
					map[byte('/')] = "\\/";
					map[byte('\b')] = "\\b";
					map[byte('\f')] = "\\f";
					map[byte('\n')] = "\\n";
					map[byte('\r')] = "\\r";
					map[byte('\t')] = "\\t";

					once = map;
				}
				std::string output;
				output.reserve(input.size() * 2 + 2); // worst scenario
				if (quote) output += '"';
				for (std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it)
					output += map[byte(*it)];
				if (quote) output += '"';
				return output;
			}


			namespace json {

				inline std::string remove_last_comma(const std::string& _input) {
					std::string input(_input);
					size_t size = input.size();
					if (size > 2)
						if (input[size - 2] == ',')
							input[size - 2] = ' ';
					return input;
				}

				inline std::string tag(unsigned format, unsigned depth, const std::string & name, const jsonxx::Value & t) {
					std::stringstream ss;
					const std::string tab(depth, '\t');

					if (!name.empty())
						ss << tab << '\"' << escape_string(name) << '\"' << ':' << ' ';
					else
						ss << tab;

					switch (t.type_)
					{
					default:
					case jsonxx::Value::NULL_:
						ss << "null";
						return ss.str() + ",\n";

					case jsonxx::Value::BOOL_:
						ss << (t.bool_value_ ? "true" : "false");
						return ss.str() + ",\n";

					case jsonxx::Value::ARRAY_:
						ss << "[\n";
						for (Array::container::const_iterator it = t.array_value_->values().begin(),
							end = t.array_value_->values().end(); it != end; ++it)
							ss << tag(format, depth + 1, std::string(), **it);
						return remove_last_comma(ss.str()) + tab + "]" ",\n";

					case jsonxx::Value::STRING_:
						ss << '\"' << escape_string(*t.string_value_) << '\"';
						return ss.str() + ",\n";

					case jsonxx::Value::OBJECT_:
						ss << "{\n";
						for (Object::container::const_iterator it = t.object_value_->kv_map().begin(),
							end = t.object_value_->kv_map().end(); it != end; ++it)
							ss << tag(format, depth + 1, it->first, *it->second);
						return remove_last_comma(ss.str()) + tab + "}" ",\n";

					case jsonxx::Value::NUMBER_:
						// max precision
						ss << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
						ss << t.number_value_;
						return ss.str() + ",\n";
					}
				}
			} // namespace jsonxx::anon::json

			namespace xml {

				inline std::string escape_attrib(const std::string& input) {
					static std::string map[256], * once = 0;
					if (!once) {
						for (int i = 0; i < 256; ++i)
							map[i] = "_";
						for (int i = int('a'); i <= int('z'); ++i)
							map[i] = std::string() + char(i);
						for (int i = int('A'); i <= int('Z'); ++i)
							map[i] = std::string() + char(i);
						for (int i = int('0'); i <= int('9'); ++i)
							map[i] = std::string() + char(i);
						once = map;
					}
					std::string output;
					output.reserve(input.size()); // worst scenario
					for (std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it)
						output += map[byte(*it)];
					return output;
				}

				inline std::string escape_tag(const std::string & input, unsigned format) {
					static std::string map[256], * once = 0;
					if (!once) {
						for (int i = 0; i < 256; ++i)
							map[i] = std::string() + char(i);
						map[byte('<')] = "&lt;";
						map[byte('>')] = "&gt;";

						switch (format)
						{
						default:
							break;

						case jsonxx::JXML:
						case jsonxx::JXMLex:
						case jsonxx::JSONx:
						case jsonxx::TaggedXML:
							map[byte('&')] = "&amp;";
							break;
						}

						once = map;
					}
					std::string output;
					output.reserve(input.size() * 5); // worst scenario
					for (std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it)
						output += map[byte(*it)];
					return output;
				}

				inline std::string open_tag(unsigned format, char type, const std::string & name, const std::string & attr = std::string(), const std::string & text = std::string()) {
					std::string tagname;
					switch (format)
					{
					default:
						return std::string();

					case jsonxx::JXML:
						if (name.empty())
							tagname = std::string("j son=\"") + type + '\"';
						else
							tagname = std::string("j son=\"") + type + ':' + escape_string(name) + '\"';
						break;

					case jsonxx::JXMLex:
						if (name.empty())
							tagname = std::string("j son=\"") + type + '\"';
						else
							tagname = std::string("j son=\"") + type + ':' + escape_string(name) + "\" " + escape_attrib(name) + "=\"" + escape_string(text) + "\"";
						break;

					case jsonxx::JSONx:
						if (!name.empty())
							tagname = std::string(" name=\"") + escape_string(name) + "\"";
						switch (type) {
						default:
						case '0': tagname = "json:null" + tagname; break;
						case 'b': tagname = "json:boolean" + tagname; break;
						case 'a': tagname = "json:array" + tagname; break;
						case 's': tagname = "json:string" + tagname; break;
						case 'o': tagname = "json:object" + tagname; break;
						case 'n': tagname = "json:number" + tagname; break;
						}
						break;

					case jsonxx::TaggedXML: // @TheMadButcher
						if (!name.empty())
							tagname = escape_attrib(name);
						else
							tagname = "JsonItem";
						switch (type) {
						default:
						case '0': tagname += " type=\"json:null\""; break;
						case 'b': tagname += " type=\"json:boolean\""; break;
						case 'a': tagname += " type=\"json:array\""; break;
						case 's': tagname += " type=\"json:string\""; break;
						case 'o': tagname += " type=\"json:object\""; break;
						case 'n': tagname += " type=\"json:number\""; break;
						}

						if (!name.empty())
							tagname += std::string(" name=\"") + escape_string(name) + "\"";

						break;
					}

					return std::string("<") + tagname + attr + ">";
				}

				inline std::string close_tag(unsigned format, char type, const std::string & name) {
					switch (format)
					{
					default:
						return std::string();

					case jsonxx::JXML:
					case jsonxx::JXMLex:
						return "</j>";

					case jsonxx::JSONx:
						switch (type) {
						default:
						case '0': return "</json:null>";
						case 'b': return "</json:boolean>";
						case 'a': return "</json:array>";
						case 'o': return "</json:object>";
						case 's': return "</json:string>";
						case 'n': return "</json:number>";
						}
						break;

					case jsonxx::TaggedXML: // @TheMadButcher
						if (!name.empty())
							return "</" + escape_attrib(name) + ">";
						else
							return "</JsonItem>";
					}
				}

				inline std::string tag(unsigned format, unsigned depth, const std::string & name, const jsonxx::Value & t, const std::string & attr = std::string()) {
					std::stringstream ss;
					const std::string tab(depth, '\t');

					switch (t.type_)
					{
					default:
					case jsonxx::Value::NULL_:
						return tab + open_tag(format, '0', name, " /") + '\n';

					case jsonxx::Value::BOOL_:
						ss << (t.bool_value_ ? "true" : "false");
						return tab + open_tag(format, 'b', name, std::string(), format == jsonxx::JXMLex ? ss.str() : std::string())
							+ ss.str()
							+ close_tag(format, 'b', name) + '\n';

					case jsonxx::Value::ARRAY_:
						for (Array::container::const_iterator it = t.array_value_->values().begin(),
							end = t.array_value_->values().end(); it != end; ++it)
							ss << tag(format, depth + 1, std::string(), **it);
						return tab + open_tag(format, 'a', name, attr) + '\n'
							+ ss.str()
							+ tab + close_tag(format, 'a', name) + '\n';

					case jsonxx::Value::STRING_:
						ss << escape_tag(*t.string_value_, format);
						return tab + open_tag(format, 's', name, std::string(), format == jsonxx::JXMLex ? ss.str() : std::string())
							+ ss.str()
							+ close_tag(format, 's', name) + '\n';

					case jsonxx::Value::OBJECT_:
						for (Object::container::const_iterator it = t.object_value_->kv_map().begin(),
							end = t.object_value_->kv_map().end(); it != end; ++it)
							ss << tag(format, depth + 1, it->first, *it->second);
						return tab + open_tag(format, 'o', name, attr) + '\n'
							+ ss.str()
							+ tab + close_tag(format, 'o', name) + '\n';

					case jsonxx::Value::NUMBER_:
						// max precision
						ss << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
						ss << t.number_value_;
						return tab + open_tag(format, 'n', name, std::string(), format == jsonxx::JXMLex ? ss.str() : std::string())
							+ ss.str()
							+ close_tag(format, 'n', name) + '\n';
					}
				}

				// order here matches jsonxx::Format enum
				static const char* defheader[] = {
					"",

					"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
					JSONXX_XML_TAG "\n",

					"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
					JSONXX_XML_TAG "\n",

					"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
					JSONXX_XML_TAG "\n",

					"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
					JSONXX_XML_TAG "\n"
				};

				// order here matches jsonxx::Format enum
				static const char* defrootattrib[] = {
					"",

					" xsi:schemaLocation=\"http://www.datapower.com/schemas/json jsonx.xsd\""
					" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
					" xmlns:json=\"http://www.ibm.com/xmlns/prod/2009/jsonx\"",

					"",

					"",

					""
				};

			} // namespace jsonxx::anon::xml

		} // namespace jsonxx::anon

		inline std::string Object::json() const {
			using namespace json;

			jsonxx::Value v;
			v.object_value_ = const_cast<jsonxx::Object*>(this);
			v.type_ = jsonxx::Value::OBJECT_;

			std::string result = tag(jsonxx::JSON, 0, std::string(), v);

			v.object_value_ = 0;
			return remove_last_comma(result);
		}

		inline std::string Object::xml(unsigned format, const std::string & header, const std::string & attrib) const {
			using namespace xml;
			JSONXX_ASSERT(format == jsonxx::JSONx || format == jsonxx::JXML || format == jsonxx::JXMLex || format == jsonxx::TaggedXML);

			jsonxx::Value v;
			v.object_value_ = const_cast<jsonxx::Object*>(this);
			v.type_ = jsonxx::Value::OBJECT_;

			std::string result = tag(format, 0, std::string(), v, attrib.empty() ? std::string(defrootattrib[format]) : attrib);

			v.object_value_ = 0;
			return (header.empty() ? std::string(defheader[format]) : header) + result;
		}

		inline std::string Array::json() const {
			using namespace json;

			jsonxx::Value v;
			v.array_value_ = const_cast<jsonxx::Array*>(this);
			v.type_ = jsonxx::Value::ARRAY_;

			std::string result = tag(jsonxx::JSON, 0, std::string(), v);

			v.array_value_ = 0;
			return remove_last_comma(result);
		}

		inline std::string Array::xml(unsigned format, const std::string & header, const std::string & attrib) const {
			using namespace xml;
			JSONXX_ASSERT(format == jsonxx::JSONx || format == jsonxx::JXML || format == jsonxx::JXMLex || format == jsonxx::TaggedXML);

			jsonxx::Value v;
			v.array_value_ = const_cast<jsonxx::Array*>(this);
			v.type_ = jsonxx::Value::ARRAY_;

			std::string result = tag(format, 0, std::string(), v, attrib.empty() ? std::string(defrootattrib[format]) : attrib);

			v.array_value_ = 0;
			return (header.empty() ? std::string(defheader[format]) : header) + result;
		}

		inline bool validate(std::istream & input) {

			// trim non-printable chars
			for (char ch(0); !input.eof() && input.peek() <= 32; )
				input.get(ch);

			// validate json
			if (input.peek() == '{')
			{
				jsonxx::Object o;
				if (parse_object(input, o))
					return true;
			}
			else
				if (input.peek() == '[')
				{
					jsonxx::Array a;
					if (parse_array(input, a))
						return true;
				}

			// bad json input
			return false;
		}

		inline bool validate(const std::string & input) {
			std::istringstream is(input);
			return jsonxx::validate(is);
		}

		inline std::string reformat(std::istream & input) {

			// trim non-printable chars
			for (char ch(0); !input.eof() && input.peek() <= 32; )
				input.get(ch);

			// validate json
			if (input.peek() == '{')
			{
				jsonxx::Object o;
				if (parse_object(input, o))
					return o.json();
			}
			else
				if (input.peek() == '[')
				{
					jsonxx::Array a;
					if (parse_array(input, a))
						return a.json();
				}

			// bad json input
			return std::string();
		}

		inline std::string reformat(const std::string & input) {
			std::istringstream is(input);
			return jsonxx::reformat(is);
		}

		inline std::string xml(std::istream & input, unsigned format) {
			using namespace xml;
			JSONXX_ASSERT(format == jsonxx::JSONx || format == jsonxx::JXML || format == jsonxx::JXMLex || format == jsonxx::TaggedXML);

			// trim non-printable chars
			for (char ch(0); !input.eof() && input.peek() <= 32; )
				input.get(ch);

			// validate json, then transform
			if (input.peek() == '{')
			{
				jsonxx::Object o;
				if (parse_object(input, o))
					return o.xml(format);
			}
			else
				if (input.peek() == '[')
				{
					jsonxx::Array a;
					if (parse_array(input, a))
						return a.xml(format);
				}

			// bad json, return empty xml
			return defheader[format];
		}

		inline std::string xml(const std::string & input, unsigned format) {
			std::istringstream is(input);
			return jsonxx::xml(is, format);
		}


		inline Object::Object(const Object & other) {
			import(other);
		}
		inline Object::Object(const std::string & key, const Value & value) {
			import(key, value);
		}
		inline void Object::import(const Object & other) {
			odd.clear();
			if (this != &other) {
				// default
				container::const_iterator
					it = other.value_map_.begin(),
					end = other.value_map_.end();
				for (/**/; it != end; ++it) {
					container::iterator found = value_map_.find(it->first);
					if (found != value_map_.end()) {
						delete found->second;
					}
					value_map_[it->first] = new Value(*it->second);
				}
			}
			else {
				// recursion is supported here
				import(Object(*this));
			}
		}
		inline void Object::import(const std::string & key, const Value & value) {
			odd.clear();
			container::iterator found = value_map_.find(key);
			if (found != value_map_.end()) {
				delete found->second;
			}
			value_map_[key] = new Value(value);
		}
		inline Object& Object::operator=(const Object & other) {
			odd.clear();
			if (this != &other) {
				reset();
				import(other);
			}
			return *this;
		}
		inline Object& Object::operator<<(const Value & value) {
			if (odd.empty()) {
				odd = value.get<String>();
			}
			else {
				import(Object(odd, value));
				odd.clear();
			}
			return *this;
		}
		inline Object& Object::operator<<(const Object & value) {
			import(std::string(odd), value);
			odd.clear();
			return *this;
		}
		inline size_t Object::size() const {
			return value_map_.size();
		}
		inline bool Object::empty() const {
			return value_map_.size() == 0;
		}
		inline const std::map<std::string, Value*>& Object::kv_map() const {
			return value_map_;
		}
		inline std::string Object::write(unsigned format) const {
			return format == JSON ? json() : xml(format);
		}
		inline void Object::reset() {
			container::iterator i;
			for (i = value_map_.begin(); i != value_map_.end(); ++i) {
				delete i->second;
			}
			value_map_.clear();
		}
		inline bool Object::parse(std::istream & input) {
			return parse(input, *this);
		}
		inline bool Object::parse(const std::string & input) {
			std::istringstream is(input);
			return parse(is, *this);
		}


		inline Array::Array(const Array & other) {
			import(other);
		}
		inline Array::Array(const Value & value) {
			import(value);
		}
		inline void Array::import(const Array & other) {
			if (this != &other) {
				// default
				container::const_iterator
					it = other.values_.begin(),
					end = other.values_.end();
				for (/**/; it != end; ++it) {
					values_.push_back(new Value(**it));
				}
			}
			else {
				// recursion is supported here
				import(Array(*this));
			}
		}
		inline void Array::import(const Value & value) {
			values_.push_back(new Value(value));
		}
		inline size_t Array::size() const {
			return values_.size();
		}
		inline bool Array::empty() const {
			return values_.size() == 0;
		}
		inline void Array::reset() {
			for (container::iterator i = values_.begin(); i != values_.end(); ++i) {
				delete* i;
			}
			values_.clear();
		}
		inline bool Array::parse(std::istream & input) {
			return parse(input, *this);
		}
		inline bool Array::parse(const std::string & input) {
			std::istringstream is(input);
			return parse(is, *this);
		}
		inline Array& Array::operator<<(const Array & other) {
			import(other);
			return *this;
		}
		inline Array& Array::operator<<(const Value & value) {
			import(value);
			return *this;
		}
		inline Array& Array::operator=(const Array & other) {
			if (this != &other) {
				reset();
				import(other);
			}
			return *this;
		}
		inline Array& Array::operator=(const Value & value) {
			reset();
			import(value);
			return *this;
		}

		inline Value::Value(const Value & other) : type_(INVALID_) {
			import(other);
		}
		inline bool Value::empty() const {
			if (type_ == INVALID_) return true;
			if (type_ == STRING_ && string_value_ == 0) return true;
			if (type_ == ARRAY_ && array_value_ == 0) return true;
			if (type_ == OBJECT_ && object_value_ == 0) return true;
			return false;
		}
		inline bool Value::parse(std::istream & input) {
			return parse(input, *this);
		}
		inline bool Value::parse(const std::string & input) {
			std::istringstream is(input);
			return parse(is, *this);
		}

	}  // namespace jsonxx


	// XSOCKET

	namespace XSOCKETN
	{
		using namespace std;
		class XSOCKET
		{
		protected:
			SOCKET X = 0;
			int fam;
			shared_ptr<int> ptr;


		public:



			bool Valid()
			{
				if (X == 0 || X == -1)
					return false;
				return true;
			}

			void CloseIf()
			{
				if (ptr.use_count() == 1)
					Close();
			}

			SOCKET h()
			{
				return X;
			}

			operator SOCKET()
			{
				return X;
			}

			~XSOCKET()
			{
				CloseIf();
			}


			XSOCKET(SOCKET t)
			{
				operator =(t);
			}

			void operator =(SOCKET t)
			{
				Close();
				X = t;
				ptr = make_shared<int>(int(0));
			}


			XSOCKET(const XSOCKET & x)
			{
				operator =(x);
			}

			XSOCKET& operator =(const XSOCKET & x)
			{
				CloseIf();
				ptr = x.ptr;
				X = x.X;
				return *this;
			}


			XSOCKET(int af = AF_INET, int ty = SOCK_STREAM, int pro = IPPROTO_TCP)
			{
				if (af == 0)
					return;
				Create(af, ty, pro);
			}

			void Create(int af = AF_INET, int ty = SOCK_STREAM, int pro = IPPROTO_TCP)
			{
				fam = af;
				X = socket(af, ty, pro);
				if (X == 0 || X == INVALID_SOCKET)
				{
					X = 0;
					return;
				}
				if (af == AF_INET6)
				{
					DWORD ag = 0;
					setsockopt(X, IPPROTO_IPV6, IPV6_V6ONLY, (char*)& ag, 4);
				}
				ptr = make_shared<int>(int(0));
			}

			void Detach()
			{
				X = 0;
			}

			void Close()
			{
				if (X != INVALID_SOCKET && X != 0)
					closesocket(X);
				X = 0;
			}

			bool Bind(int port)
			{
				if (fam == AF_INET6)
				{
					sockaddr_in6 sA = { 0 };
					sA.sin6_family = (ADDRESS_FAMILY)fam;
					sA.sin6_port = (u_short)htons((u_short)port);
					if (::bind(X, (sockaddr*)& sA, sizeof(sA)) < 0)
						return false;

				}
				else
				{
					sockaddr_in sA = { 0 };
					sA.sin_addr.s_addr = INADDR_ANY;
					sA.sin_family = (ADDRESS_FAMILY)fam;
					sA.sin_port = (u_short)htons((u_short)port);
					if (::bind(X, (sockaddr*)& sA, sizeof(sA)) < 0)
						return false;
				}
				return true;
			}

			bool BindAndListen(int port)
			{
				if (!Bind(port))
					return false;
				listen(X, 3);
				return true;
			}

			SOCKET Accept()
			{
				return accept(X, 0, 0);
			}


			BOOL ConnectTo(const char* addr, int port, int sec = 0, std::tuple<wstring, int, int> proxy = make_tuple<>(L"", 0, 0))
			{
				// Check the address
				if (!addr || !port)
					return false;
				wchar_t se[100] = { 0 };
				swprintf_s(se, 100, L"%u", port);
				timeval tv = { 0 };
				tv.tv_sec = sec;
				wchar_t adr[1000] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, addr, -1, adr, 1000);
				//		wcscpy_s(adr, 1000, UWL::ystring(addr));
				if (std::get<1>(proxy) == 0)
					return WSAConnectByName(X, adr, se, 0, 0, 0, 0, sec ? &tv : 0, 0);

				swprintf_s(se, 100, L"%u", std::get<1>(proxy));
				BOOL Rx = WSAConnectByName(X, (LPWSTR)std::get<0>(proxy).c_str(), se, 0, 0, 0, 0, sec ? &tv : 0, 0);
				if (!Rx)
					return FALSE;

				// Try the proxy connection
				if (std::get<2>(proxy) == 5)
				{
					// SOCKS 5
					// A - Send 0x05 0x02 0x00 0x02
					//     or 0x05 0x01 0x00 if we have no uid/pwd
					char* a1 = "\x05\x01\x00";
					transmit(a1, 3, true);
					char r1[2];
					receive(r1, 2, true);
					if (r1[0] != 5) return FALSE;
					if (r1[1] != 0) return FALSE;

					// Send request
					/*
					+----+-----+-------+------+----------+----------+
					|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
					+----+-----+-------+------+----------+----------+
					| 1  |  1  | X'00' |  1   | Variable |    2     |
					+----+-----+-------+------+----------+----------+
					*/

					char b[1000] = { 0 };
					b[0] = 5;
					b[1] = 1; // Connect
					b[2] = 0; // R
					b[3] = 3; // Domain name
					b[4] = (char)strlen(addr);
					strcpy_s(b + 5, 995, addr);
					short p2 = htons((short)port);
					memcpy(b + 5 + strlen(addr), &p2, 2);

					int ts = 5 + (int)strlen(addr) + 2;
					transmit(b, ts, true);

					/*

					+----+-----+-------+------+----------+----------+
					|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
					+----+-----+-------+------+----------+----------+
					| 1  |  1  | X'00' |  1   | Variable |    2     |
					+----+-----+-------+------+----------+----------+


					*/
					char saddr[100];
					int rv = receive(saddr, 4, true);
					if (rv != 4)
						return FALSE;
					if (saddr[1] != 0)
						return FALSE;

					// Success! Get stuff out of our way
					if (saddr[3] == '\x01')
						receive(saddr, 6, true);
					else
						if (saddr[3] == '\x04')
							receive(saddr, 18, true);

					return TRUE;

				}


				/*
				//	ADDRINFO h1;
				ADDRINFO* h2 = 0;
				char po[100] = { 0 };
				sprintf_s(po, 100, "%u", port);
				getaddrinfo(addr, po, 0, &h2);
				if (!h2)
					return false;

				if (X == INVALID_SOCKET || X == 0)
					return 0;

				if (connect(X, (sockaddr*)h2->ai_addr, (int)h2->ai_addrlen) < 0)
					return false;
				return true;
				*/

				return TRUE;
			}


			int utransmit(char* b, int sz)
			{
				return sendto(X, b, sz, 0, 0, 0);
			}

			int ureceive(char* b, int sz)
			{
				return recvfrom(X, b, sz, 0, 0, 0);
			}

			int transmit(char* b, int sz, bool ForceAll = false, int p = 0, std::function<void(int)> cb = nullptr)
			{
				// same as send, but forces reading ALL sz
				if (!ForceAll)
					return send(X, b, sz, p);
				int rs = 0;
				for (;;)
				{
					int tosend = sz - rs;
					if (tosend > 10000)
						tosend = 10000;
					int rval = send(X, b + rs, tosend, p);
					if (rval == 0 || rval == SOCKET_ERROR)
						return rs;
					rs += rval;
					if (cb)
						cb(rs);
					if (rs == sz)
						return rs;
				}
			}


			int receive(char* b, int sz, bool ForceAll = false, int p = 0)
			{
				// same as recv, but forces reading ALL sz
				if (!ForceAll)
					return recv(X, b, sz, p);
				int rs = 0;
				for (;;)
				{
					int rval = recv(X, b + rs, sz - rs, p);
					if (rval == 0 || rval == SOCKET_ERROR)
						return rs;
					rs += rval;
					if (rs == sz)
						return rs;
				}
			}





		};

		typedef void(__stdcall * sscb)(SOCKET X, unsigned long long cur, unsigned long long max, unsigned long long lparam);
		class SSL_SOCKET_CALLBACK
		{
		public:

			sscb scb = 0;
			unsigned long long lparam = 0;
		};

		class SSL_SOCKET : public XSOCKET
		{
		private:

			int Type = 0;
			HCERTSTORE hCS = 0;
			SCHANNEL_CRED m_SchannelCred;
			CredHandle hCred;
			CtxtHandle hCtx;
			wstring dn;
			SecBufferDesc sbin;
			SecBufferDesc sbout;
			bool InitContext = 0;
			vector<char> ExtraData;
			vector<char> PendingRecvData;
			PCCERT_CONTEXT OurCertificate = 0;
			bool IsExternalCert = 0;


		public:

			void SetType(int ty)
			{
				Type = ty;
			}

			SSL_SOCKET(int af = AF_INET, int ty = 0, PCCERT_CONTEXT pc = 0) : XSOCKET(af)
			{
				Type = ty;
				hCred.dwLower = 0;
				hCred.dwUpper = 0;
				hCtx.dwLower = 0;
				hCtx.dwUpper = 0;
				if (pc)
				{
					OurCertificate = pc;
					IsExternalCert = true;
				}
			}

			~SSL_SOCKET()
			{
				if (Type == 0)
					ClientOff();
				else
					ServerOff();

				if (hCtx.dwLower || hCtx.dwLower)
					DeleteSecurityContext(&hCtx);

				if (hCred.dwLower || hCred.dwLower)
					FreeCredentialHandle(&hCred);

				if (OurCertificate && !IsExternalCert)
				{
					CertFreeCertificateContext(OurCertificate);
					OurCertificate = 0;
				}

				if (hCS)
					CertCloseStore(hCS, 0);
				hCS = 0;
			}

			void SetDestinationName(const wchar_t* n)
			{
				dn = n;
			}

			int ClientOff()
			{
				// Client wants to disconnect
				SECURITY_STATUS ss = 0;
				vector<SecBuffer> OutBuffers(100);
				DWORD dwType = SCHANNEL_SHUTDOWN;
				OutBuffers[0].pvBuffer = &dwType;
				OutBuffers[0].BufferType = SECBUFFER_TOKEN;
				OutBuffers[0].cbBuffer = sizeof(dwType);

				sbout.cBuffers = 1;
				sbout.pBuffers = OutBuffers.data();
				sbout.ulVersion = SECBUFFER_VERSION;

				for (;;)
				{
					ss = ApplyControlToken(&hCtx, &sbout);
					if (FAILED(ss))
						return -1;

					DWORD dwSSPIFlags = 0;
					DWORD dwSSPIOutFlags = 0;
					dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;

					OutBuffers[0].pvBuffer = NULL;
					OutBuffers[0].BufferType = SECBUFFER_TOKEN;
					OutBuffers[0].cbBuffer = 0;
					sbout.cBuffers = 1;
					sbout.pBuffers = OutBuffers.data();
					sbout.ulVersion = SECBUFFER_VERSION;

					ss = InitializeSecurityContext(&hCred, &hCtx, NULL, dwSSPIFlags, 0, SECURITY_NATIVE_DREP, NULL, 0, &hCtx, &sbout, &dwSSPIOutFlags, 0);
					if (FAILED(ss))
						return -1;

					PBYTE pbMessage = 0;
					DWORD cbMessage = 0;
					pbMessage = (BYTE*)(OutBuffers[0].pvBuffer);
					cbMessage = OutBuffers[0].cbBuffer;

					if (pbMessage != NULL && cbMessage != 0)
					{
						int rval = transmit((char*)pbMessage, cbMessage, true);
						FreeContextBuffer(pbMessage);
						return rval;
					}
					break;
				}
				return 1;
			}

			int ServerOff()
			{
				// Server wants to disconnect
				SECURITY_STATUS ss;
				vector<SecBuffer> OutBuffers(100);
				DWORD dwType = SCHANNEL_SHUTDOWN;
				OutBuffers[0].pvBuffer = &dwType;
				OutBuffers[0].BufferType = SECBUFFER_TOKEN;
				OutBuffers[0].cbBuffer = sizeof(dwType);

				sbout.cBuffers = 1;
				sbout.pBuffers = OutBuffers.data();
				sbout.ulVersion = SECBUFFER_VERSION;

				for (;;)
				{
					ss = ApplyControlToken(&hCtx, &sbout);
					if (FAILED(ss))
						return -1;

					DWORD dwSSPIFlags = 0;
					DWORD dwSSPIOutFlags = 0;
					dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;

					OutBuffers[0].pvBuffer = NULL;
					OutBuffers[0].BufferType = SECBUFFER_TOKEN;
					OutBuffers[0].cbBuffer = 0;
					sbout.cBuffers = 1;
					sbout.pBuffers = OutBuffers.data();
					sbout.ulVersion = SECBUFFER_VERSION;

					ss = AcceptSecurityContext(&hCred, &hCtx, NULL, dwSSPIFlags, SECURITY_NATIVE_DREP, NULL, &sbout, &dwSSPIOutFlags, 0);
					if (FAILED(ss))
						return -1;

					PBYTE pbMessage = 0;
					DWORD cbMessage = 0;
					pbMessage = (BYTE*)(OutBuffers[0].pvBuffer);
					cbMessage = OutBuffers[0].cbBuffer;

					if (pbMessage != NULL && cbMessage != 0)
					{
						int rval = transmit((char*)pbMessage, cbMessage, true);
						FreeContextBuffer(pbMessage);
						return rval;
					}
					break;
				}
				return 1;
			}

			int sreceive(char* b, int sz, int fall, SSL_SOCKET_CALLBACK* ssc)
			{
				int rs = 0;
				for (;;)
				{
					int rval = s_recv(b + rs, sz - rs);
					if (rval == 0 || rval == SOCKET_ERROR)
						return rs;
					rs += rval;
					if (ssc)
					{
						if (ssc->scb)
						{
							ssc->scb(0, rs, sz, ssc->lparam);
						}
					}
					if (rs == sz || fall == 0)
						return rs;
				}
			}


			int s_recv(char* b, unsigned  int sz, vector<char> * encr = 0)
			{
				SecPkgContext_StreamSizes Sizes;
				SECURITY_STATUS ss = 0;
				ss = QueryContextAttributes(&hCtx, SECPKG_ATTR_STREAM_SIZES, &Sizes);
				if (FAILED(ss))
					return -1;

				unsigned int TotalR = 0;
				int pI = 0;
				SecBuffer Buffers[5] = { 0 };
				SecBuffer* pDataBuffer;
				SecBuffer* pExtraBuffer;
				vector<char> mmsg(Sizes.cbMaximumMessage * 2);

				if (PendingRecvData.size())
				{
					if (sz <= PendingRecvData.size())
					{
						memcpy(b, PendingRecvData.data(), sz);
						vector<char> dj(PendingRecvData.size() - sz);
						memcpy(dj.data(), PendingRecvData.data() + sz, PendingRecvData.size() - sz);
						PendingRecvData = dj;
						return sz;
					}
					// else , occupied already
					memcpy(b, PendingRecvData.data(), PendingRecvData.size());
					sz = (unsigned int)PendingRecvData.size();
					PendingRecvData.clear();
					return sz;
				}

				for (;;)
				{
					unsigned int dwMessage = Sizes.cbMaximumMessage;

					if (dwMessage > Sizes.cbMaximumMessage)
						dwMessage = Sizes.cbMaximumMessage;

					int rval = 0;
					if (ExtraData.size())
					{
						memcpy(mmsg.data() + pI, ExtraData.data(), ExtraData.size());
						pI += (unsigned int)ExtraData.size();
						ExtraData.clear();
					}
					else
					{
						if (encr)
						{
							memcpy(mmsg.data() + pI, encr->data(), encr->size());
							rval = (int)encr->size();
						}
						else
						{
							rval = receive_raw(mmsg.data() + pI, dwMessage);
						}
						if (rval == 0 || rval == -1)
							return rval;
						pI += rval;
					}


					Buffers[0].pvBuffer = mmsg.data();
					Buffers[0].cbBuffer = pI;
					Buffers[0].BufferType = SECBUFFER_DATA;

					Buffers[1].BufferType = SECBUFFER_EMPTY;
					Buffers[2].BufferType = SECBUFFER_EMPTY;
					Buffers[3].BufferType = SECBUFFER_EMPTY;

					sbin.ulVersion = SECBUFFER_VERSION;
					sbin.pBuffers = Buffers;
					sbin.cBuffers = 4;

					ss = DecryptMessage(&hCtx, &sbin, 0, NULL);
					if (ss == SEC_E_INCOMPLETE_MESSAGE)
						continue;
					if (ss != SEC_E_OK && ss != SEC_I_RENEGOTIATE && ss != SEC_I_CONTEXT_EXPIRED)
						return -1;

					pDataBuffer = NULL;
					pExtraBuffer = NULL;
					for (int i = 0; i < 4; i++)
					{
						if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA)
						{
							pDataBuffer = &Buffers[i];
						}
						if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA)
						{
							pExtraBuffer = &Buffers[i];
						}
					}
					if (pExtraBuffer)
					{
						ExtraData.resize(pExtraBuffer->cbBuffer);
						memcpy(ExtraData.data(), pExtraBuffer->pvBuffer, ExtraData.size());
						pI = 0;
					}

					if (ss == SEC_I_RENEGOTIATE)
					{
						ss = ClientLoop();
						if (FAILED(ss))
							return -1;
					}



					if (pDataBuffer == 0)
						break;
					else if ((pDataBuffer->cbBuffer == 0) && ExtraData.size())
					{
						// BUG under Windows 7/Server 2008
						// DecryptMessage needs to be called a second time
						// in order to get the plain data
						continue;
					}

					TotalR = pDataBuffer->cbBuffer;
					if (TotalR <= sz)
					{
						memcpy(b, pDataBuffer->pvBuffer, TotalR);
					}
					else
					{
						TotalR = sz;
						memcpy(b, pDataBuffer->pvBuffer, TotalR);
						PendingRecvData.resize(pDataBuffer->cbBuffer - TotalR);
						memcpy(PendingRecvData.data(), (char*)pDataBuffer->pvBuffer + TotalR, PendingRecvData.size());
					}


					break;
				}



				return TotalR;
			}

			int transmit_raw(char* b, int sz, bool ForceAll = false, int p = 0, std::function<void(int)> cb = nullptr)
			{
				return XSOCKET::transmit(b, sz, ForceAll, p, cb);
			}

			int transmit(char* b, int sz, bool ForceAll = false, int p = 0, std::function<void(int)> cb = nullptr)
			{
				UNREFERENCED_PARAMETER(p);
				UNREFERENCED_PARAMETER(ForceAll);
				return stransmit(b, sz);
			}

			int receive(char* b, int sz, bool ForceAll = false, int p = 0)
			{
				UNREFERENCED_PARAMETER(p);
				UNREFERENCED_PARAMETER(ForceAll);
				return sreceive(b, sz, ForceAll, 0);
			}

			int receive_raw(char* b, int sz, bool ForceAll = false, int p = 0)
			{
				return XSOCKET::receive(b, sz, ForceAll, p);
			}

			int stransmit(char* b, int sz, vector<char> * rm = 0)
			{
				// QueryContextAttributes
				// Encrypt Message
				// ssend

				SecPkgContext_StreamSizes Sizes = { 0 };
				SECURITY_STATUS ss = 0;
				ss = QueryContextAttributes(&hCtx, SECPKG_ATTR_STREAM_SIZES, &Sizes);
				if (FAILED(ss))
					return -1;

				vector<SecBuffer> Buffers(100);
				int mPos = 0;
				for (;;)
				{
					vector<char> mmsg(Sizes.cbMaximumMessage * 2);
					vector<char> mhdr(Sizes.cbHeader * 2);
					vector<char> mtrl(Sizes.cbTrailer * 2);

					unsigned int dwMessage = sz - mPos;
					if (dwMessage == 0)
						break; // all ok!

					if (dwMessage > Sizes.cbMaximumMessage)
					{
						dwMessage = Sizes.cbMaximumMessage;
					}
					memcpy(mmsg.data(), b + mPos, dwMessage);
					mPos += dwMessage;


					Buffers[0].pvBuffer = mhdr.data();
					Buffers[0].cbBuffer = Sizes.cbHeader;
					Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
					Buffers[2].pvBuffer = mtrl.data();
					Buffers[2].cbBuffer = Sizes.cbTrailer;
					Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
					Buffers[3].pvBuffer = 0;
					Buffers[3].cbBuffer = 0;
					Buffers[3].BufferType = SECBUFFER_EMPTY;
					Buffers[1].pvBuffer = mmsg.data();
					Buffers[1].cbBuffer = dwMessage;
					Buffers[1].BufferType = SECBUFFER_DATA;

					sbin.ulVersion = SECBUFFER_VERSION;
					sbin.pBuffers = Buffers.data();
					sbin.cBuffers = 4;

					ss = EncryptMessage(&hCtx, 0, &sbin, 0);
					if (FAILED(ss))
						return -1;

					if (rm)
					{
						int Total = Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer;
						rm->resize(Total);
						int c = 0;
						memcpy(rm->data() + c, Buffers[0].pvBuffer, Buffers[0].cbBuffer);
						c += Buffers[0].cbBuffer;
						memcpy(rm->data() + c, Buffers[1].pvBuffer, Buffers[1].cbBuffer);
						c += Buffers[1].cbBuffer;
						memcpy(rm->data() + c, Buffers[2].pvBuffer, Buffers[2].cbBuffer);
					}
					else
					{
						// Send this message
						unsigned int rval = (unsigned int)transmit_raw((char*)Buffers[0].pvBuffer, Buffers[0].cbBuffer, true);
						if (rval != Buffers[0].cbBuffer)
							return rval;
						rval = transmit_raw((char*)Buffers[1].pvBuffer, Buffers[1].cbBuffer, true);
						if (rval != Buffers[1].cbBuffer)
							return rval;
						rval = transmit_raw((char*)Buffers[2].pvBuffer, Buffers[2].cbBuffer, true);
						if (rval != Buffers[2].cbBuffer)
							return rval;
					}
				}


				return sz;
			}



			int ClientLoop()
			{
				SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;
				vector<char> t(0x11000);
				vector<SecBuffer> bufsi(100);
				vector<SecBuffer> bufso(100);
				int pt = 0;

				// Loop using InitializeSecurityContext until success
				for (;;)
				{
					if (ss != SEC_I_CONTINUE_NEEDED && ss != SEC_E_INCOMPLETE_MESSAGE && ss != SEC_I_INCOMPLETE_CREDENTIALS)
						break;

					DWORD dwSSPIFlags =
						ISC_REQ_SEQUENCE_DETECT |
						ISC_REQ_REPLAY_DETECT |
						ISC_REQ_CONFIDENTIALITY |
						ISC_RET_EXTENDED_ERROR |
						ISC_REQ_ALLOCATE_MEMORY |
						ISC_REQ_STREAM;

					dwSSPIFlags |= ISC_REQ_MANUAL_CRED_VALIDATION;

					if (InitContext == 0)
					{
						// Initialize sbout
						bufso[0].pvBuffer = NULL;
						bufso[0].BufferType = SECBUFFER_TOKEN;
						bufso[0].cbBuffer = 0;
						sbout.ulVersion = SECBUFFER_VERSION;
						sbout.cBuffers = 1;
						sbout.pBuffers = bufso.data();
					}
					else
					{
						// Get Some data from the remote site

						// Add also extradata?
						if (ExtraData.size())
						{
							memcpy(t.data(), ExtraData.data(), ExtraData.size());
							pt += (unsigned int)ExtraData.size();
							ExtraData.clear();
						}


						int rval = recv(X, t.data() + pt, 0x10000, 0);
						if (rval == 0 || rval == -1)
							return rval;
						pt += rval;

						// Put this data into the buffer so InitializeSecurityContext will do

						bufsi[0].BufferType = SECBUFFER_TOKEN;
						bufsi[0].cbBuffer = pt;
						bufsi[0].pvBuffer = t.data();
						bufsi[1].BufferType = SECBUFFER_EMPTY;
						bufsi[1].cbBuffer = 0;
						bufsi[1].pvBuffer = 0;
						sbin.ulVersion = SECBUFFER_VERSION;
						sbin.pBuffers = bufsi.data();
						sbin.cBuffers = 2;

						bufso[0].pvBuffer = NULL;
						bufso[0].BufferType = SECBUFFER_TOKEN;
						bufso[0].cbBuffer = 0;
						sbout.cBuffers = 1;
						sbout.pBuffers = bufso.data();
						sbout.ulVersion = SECBUFFER_VERSION;

					}

					DWORD dwSSPIOutFlags = 0;

					SEC_E_INTERNAL_ERROR;
					ss = InitializeSecurityContext(
						&hCred,
						InitContext ? &hCtx : 0,
						(SEC_WCHAR*)dn.c_str(),
						dwSSPIFlags,
						0,
						0,//SECURITY_NATIVE_DREP,
						InitContext ? &sbin : 0,
						0,
						InitContext ? 0 : &hCtx,
						&sbout,
						&dwSSPIOutFlags,
						0);

					if (ss == SEC_E_INCOMPLETE_MESSAGE)
						continue; // allow more

					pt = 0;

					if (FAILED(ss))
						return -1;

					if (InitContext == 0 && ss != SEC_I_CONTINUE_NEEDED)
						return -1;

					// Handle possible ExtraData
					if (bufsi[1].BufferType == SECBUFFER_EXTRA)
					{
						SecBuffer& bu = bufsi[1];
						SecBuffer& bu0 = bufsi[0];
						ExtraData.resize(bu.cbBuffer);

						if (bu.pvBuffer)
							memcpy(ExtraData.data(), bu.pvBuffer, ExtraData.size());
						else
							if (bu0.pvBuffer && bu0.cbBuffer >= ExtraData.size())
							{
								memcpy(ExtraData.data(), (char*)bu0.pvBuffer + (bu0.cbBuffer - ExtraData.size()), ExtraData.size());
							}
					}


					if (!InitContext)
					{
						// Send the data we got to the remote part
						//cbData = Send(OutBuffers[0].pvBuffer,OutBuffers[0].cbBuffer);
						unsigned int rval = (unsigned int)transmit_raw((char*)bufso[0].pvBuffer, bufso[0].cbBuffer, true);
						FreeContextBuffer(bufso[0].pvBuffer);
						if (rval != bufso[0].cbBuffer)
							return -1;
						InitContext = true;
						continue;
					}

					// Pass data to the remote site
					unsigned int rval = (unsigned int)transmit_raw((char*)bufso[0].pvBuffer, bufso[0].cbBuffer, true);
					FreeContextBuffer(bufso[0].pvBuffer);
					if (rval != bufso[0].cbBuffer)
						return -1;


					if (ss == S_OK)
						break; // wow!!

				}
				return 0;
			}



			void NoFail(HRESULT hr)
			{
				if (FAILED(hr))
					throw;
			}

			PCCERT_CONTEXT CreateOurCertificate()
			{
				// CertCreateSelfSignCertificate(0,&SubjectName,0,0,0,0,0,0);
				HRESULT hr = 0;
				HCRYPTPROV hProv = NULL;
				PCCERT_CONTEXT p = 0;
				HCRYPTKEY hKey = 0;
				CERT_NAME_BLOB sib = { 0 };
				BOOL AX = 0;

				// Step by step to create our own certificate
				try
				{
					// Create the subject
					char cb[1000] = { 0 };
					sib.pbData = (BYTE*)cb;
					sib.cbData = 1000;
					wchar_t* szSubject = L"CN=Certificate";
					if (!CertStrToName(CRYPT_ASN_ENCODING, szSubject, 0, 0, sib.pbData, &sib.cbData, NULL))
						throw;


					// Acquire Context
					wchar_t* pszKeyContainerName = L"Container";

					if (!CryptAcquireContext(&hProv, pszKeyContainerName, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET))
					{
						hr = GetLastError();
						if (GetLastError() == NTE_EXISTS)
						{
							if (!CryptAcquireContext(&hProv, pszKeyContainerName, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET))
							{
								throw;
							}
						}
						else
							throw;
					}

					// Generate KeyPair
					if (!CryptGenKey(hProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hKey))
						throw;

					// Generate the certificate
					CRYPT_KEY_PROV_INFO kpi = { 0 };
					kpi.pwszContainerName = pszKeyContainerName;
					kpi.pwszProvName = MS_DEF_PROV;
					kpi.dwProvType = PROV_RSA_FULL;
					kpi.dwFlags = CERT_SET_KEY_CONTEXT_PROP_ID;
					kpi.dwKeySpec = AT_KEYEXCHANGE;

					SYSTEMTIME et;
					GetSystemTime(&et);
					et.wYear += 1;

					CERT_EXTENSIONS exts = { 0 };
					p = CertCreateSelfSignCertificate(hProv, &sib, 0, &kpi, NULL, NULL, &et, &exts);

					if (p)
						AX = CryptFindCertificateKeyProvInfo(p, CRYPT_FIND_MACHINE_KEYSET_FLAG, NULL);
				}

				catch (...)
				{
				}

				if (hKey)
					CryptDestroyKey(hKey);
				hKey = 0;

				if (hProv)
					CryptReleaseContext(hProv, 0);
				hProv = 0;
				return p;
			}

			int ServerInit(bool NoLoop = false)
			{
				SECURITY_STATUS ss = 0;
				if (IsExternalCert)
				{
					;
				}
				else
				{
					//BOOL AX;
					OurCertificate = CreateOurCertificate();
				}

				// Configure our SSL SChannel
				memset(&m_SchannelCred, 0, sizeof(m_SchannelCred));
				m_SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
				m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
				m_SchannelCred.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SYSTEM_MAPPER | SCH_CRED_REVOCATION_CHECK_CHAIN;
				m_SchannelCred.hRootStore = hCS;
				m_SchannelCred.dwMinimumCipherStrength = 128;

				if (OurCertificate)
				{
					m_SchannelCred.cCreds = 1;
					m_SchannelCred.paCred = &OurCertificate;
				}

				ss = AcquireCredentialsHandle(0, SCHANNEL_NAME, SECPKG_CRED_INBOUND, 0, &m_SchannelCred, 0, 0, &hCred, 0);
				if (FAILED(ss))
					return -1;
				if (NoLoop)
					return 0;
				return ServerLoop();
			}

			int ClientInit(bool NoLoop = false)
			{
				SECURITY_STATUS ss = 0;
				if (IsExternalCert)
				{
					;
				}
				else
				{
					OurCertificate = CreateOurCertificate();
				}

				// Configure our SSL SChannel
				memset(&m_SchannelCred, 0, sizeof(m_SchannelCred));
				m_SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
				m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
				m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SYSTEM_MAPPER | SCH_CRED_REVOCATION_CHECK_CHAIN;

				if (OurCertificate)
				{
					m_SchannelCred.cCreds = 1;
					m_SchannelCred.paCred = &OurCertificate;
				}

				ss = AcquireCredentialsHandle(0, SCHANNEL_NAME, SECPKG_CRED_OUTBOUND, 0, &m_SchannelCred, 0, 0, &hCred, 0);
				if (FAILED(ss))
					return 0;

				if (NoLoop)
					return 0;
				return ClientLoop();
			}

			int ServerLoop()
			{
				// Loop AcceptSecurityContext
				SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;
				vector<char> t(0x11000);
				vector<SecBuffer> bufsi(100);
				vector<SecBuffer> bufso(100);
				int pt = 0;

				// Loop using InitializeSecurityContext until success
				for (;;)
				{
					if (ss != SEC_I_CONTINUE_NEEDED && ss != SEC_E_INCOMPLETE_MESSAGE && ss != SEC_I_INCOMPLETE_CREDENTIALS)
						break;

					DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
						ISC_REQ_REPLAY_DETECT |
						ISC_REQ_CONFIDENTIALITY |
						ISC_RET_EXTENDED_ERROR |
						ISC_REQ_ALLOCATE_MEMORY |
						ISC_REQ_STREAM;

					dwSSPIFlags |= ISC_REQ_MANUAL_CRED_VALIDATION;

					// Get Some data from the remote site
					int rval = recv(X, t.data() + pt, 0x10000, 0);
					if (rval == 0 || rval == -1)
						return -1;
					pt += rval;

					// Put this data into the buffer so InitializeSecurityContext will do
					bufsi[0].BufferType = SECBUFFER_TOKEN;
					bufsi[0].cbBuffer = pt;
					bufsi[0].pvBuffer = t.data();
					bufsi[1].BufferType = SECBUFFER_EMPTY;
					bufsi[1].cbBuffer = 0;
					bufsi[1].pvBuffer = 0;
					sbin.ulVersion = SECBUFFER_VERSION;
					sbin.pBuffers = bufsi.data();
					sbin.cBuffers = 2;

					bufso[0].pvBuffer = NULL;
					bufso[0].BufferType = SECBUFFER_TOKEN;
					bufso[0].cbBuffer = 0;
					bufso[1].BufferType = SECBUFFER_EMPTY;
					bufso[1].cbBuffer = 0;
					bufso[1].pvBuffer = 0;
					sbout.cBuffers = 2;
					sbout.pBuffers = bufso.data();
					sbout.ulVersion = SECBUFFER_VERSION;


					SEC_E_INTERNAL_ERROR;
					DWORD flg = 0;
					ss = AcceptSecurityContext(
						&hCred,
						InitContext ? &hCtx : 0,
						&sbin,
						ASC_REQ_ALLOCATE_MEMORY, 0,
						InitContext ? 0 : &hCtx,
						&sbout,
						&flg,
						0);

					InitContext = true;

					if (ss == SEC_E_INCOMPLETE_MESSAGE)
						continue; // allow more

					pt = 0;

					if (FAILED(ss))
						return -1;

					if (InitContext == 0 && ss != SEC_I_CONTINUE_NEEDED)
						return -1;

					// Pass data to the remote site
					unsigned int rval2 = (unsigned int)transmit((char*)bufso[0].pvBuffer, bufso[0].cbBuffer, true);
					FreeContextBuffer(bufso[0].pvBuffer);
					if (rval2 != bufso[0].cbBuffer)
						return -1;

					if (ss == S_OK)
						break; // wow!!

				}
				return 0;
			}





		};


		// Socket stuff
		inline int rrecv(SOCKET s, char* b, int sz, int p = 0)
		{
			// same as recv, but forces reading ALL sz
			int rs = 0;
			for (;;)
			{
				int rval = recv(s, b + rs, sz - rs, p);
				if (rval == 0 || rval == SOCKET_ERROR)
					return rs;
				rs += rval;
				if (rs == sz)
					return rs;
			}
		}

		inline int ssend(SOCKET s, char* b, int sz, int p = 0)
		{
			// same as send, but forces reading ALL sz
			int rs = 0;
			for (;;)
			{
				int tosend = sz - rs;
				if (tosend > 10000)
					tosend = 10000;
				int rval = send(s, b + rs, tosend, p);
				if (rval == 0 || rval == SOCKET_ERROR)
					return rs;
				rs += rval;
				if (rs == sz)
					return rs;
			}
		}

	};


	// REST and GOD

	namespace RESTAPI {

		using namespace std;
		typedef vector<char> DATA;


		inline std::vector<std::wstring>& split(const std::wstring& s, wchar_t delim, std::vector<std::wstring>& elems) {
			std::wstringstream ss(s);
			std::wstring item;
			while (std::getline(ss, item, delim))
			{
				elems.push_back(item);
			}
			return elems;
		}


		inline std::vector<std::wstring> split(const std::wstring& s, wchar_t delim) {
			std::vector<std::wstring> elems;
			split(s, delim, elems);
			return elems;
		}


		class data_provider
		{
		public:

			virtual size_t s() = 0;
			virtual size_t Read(char* Buff, size_t sz) = 0;
			virtual bool CanOnce() = 0;
			virtual std::tuple<const char*, size_t> Once() = 0;

		};

		class memory_data_provider : public data_provider
		{
		private:
			size_t size;
			const char* ptr;
			size_t fp = 0;
		public:

			memory_data_provider(const char* a, size_t sz)
			{
				fp = 0;
				ptr = a;
				size = sz;
			}

			virtual size_t s()
			{
				return size;
			}

			virtual size_t Read(char* Buff, size_t sz)
			{
				size_t av = size - fp;
				if (av < sz)
					sz = av;
				memcpy(Buff, ptr + fp, sz);
				fp += sz;
				return sz;
			}

			virtual bool CanOnce()
			{
				return true;
			}
			virtual std::tuple<const char*, size_t> Once()
			{
				return make_tuple<const char*, size_t>((const char*&&)ptr, (size_t&&)size);
			}


		};

		class file_data_provider : public data_provider
		{
		private:
			HANDLE hY;
		public:

			file_data_provider(HANDLE h)
			{
				hY = h;
			}

			virtual size_t s()
			{
				LARGE_INTEGER li;
				GetFileSizeEx(hY, &li);
				return (size_t)li.QuadPart;
			}

			virtual size_t Read(char* Buff, size_t sz)
			{
				DWORD A = 0;
				if (sz > 4294967296LL)
					sz = 4 * 1024 * 1024;
				if (!ReadFile(hY, Buff, (DWORD)sz, &A, 0))
					return 0;
				return A;
			}


			virtual bool CanOnce()
			{
				return false;
			}
			virtual std::tuple<const char*, size_t> Once()
			{
				return make_tuple<const char*, size_t>(0, 0);
			}
		};


		class data_writer
		{
		public:

			virtual DWORD Write(const char* Buff, DWORD sz) = 0;
			virtual size_t s() = 0;

		};

		class memory_data_writer : public data_writer
		{
		private:
			vector<char> p;
		public:

			virtual size_t s()
			{
				return p.size();
			}

			virtual DWORD Write(const char* Buff, DWORD sz)
			{
				auto s1 = p.size();
				p.resize(s1 + sz);
				memcpy(p.data() + s1, Buff, sz);
				return sz;
			}

			vector<char>& GetP() { return p; }

		};

		class file_data_writer : public data_writer
		{
		private:
			HANDLE hY;
		public:

			file_data_writer(HANDLE h)
			{
				hY = h;
			}

			virtual size_t s()
			{
				LARGE_INTEGER li;
				GetFileSizeEx(hY, &li);
				return (size_t)li.QuadPart;
			}

			virtual DWORD Write(const char* Buff, DWORD sz)
			{
				DWORD A = 0;
				if (!WriteFile(hY, Buff, sz, &A, 0))
					return 0;
				return A;
			}
		};



		class ihandle
		{
		private:
			HINTERNET hX = 0;
			std::shared_ptr<size_t> ptr = std::make_shared<size_t>();

		public:

			// Closing items
			void Close()
			{
				if (!ptr || ptr.use_count() > 1)
				{
					ptr.reset();
					return;
				}
				ptr.reset();
				if (hX != 0)
					InternetCloseHandle(hX);
				hX = 0;
			}

			ihandle()
			{
				hX = 0;
			}
			~ihandle()
			{
				Close();
			}
			ihandle(const ihandle & h)
			{
				Dup(h);
			}
			ihandle(ihandle && h)
			{
				Move(std::forward<ihandle>(h));
			}
			ihandle(HINTERNET hY)
			{
				hX = hY;
			}
			ihandle& operator =(const ihandle & h)
			{
				Dup(h);
				return *this;
			}
			ihandle& operator =(ihandle && h)
			{
				Move(std::forward<ihandle>(h));
				return *this;
			}

			void Dup(const ihandle & h)
			{
				Close();
				hX = h.hX;
				ptr = h.ptr;
			}
			void Move(ihandle && h)
			{
				Close();
				hX = h.hX;
				ptr = h.ptr;
				h.ptr.reset();
				h.hX = 0;
			}
			operator HINTERNET() const
			{
				return hX;
			}


		};

		class REST
		{
		private:

			wstring Agent = L"REST";
			ihandle hI;
			ihandle hI2;
			wstring Host;
			bool ssl = false;

		public:

			REST(const wchar_t* ag = 0)
			{
				if (ag)
					Agent = ag;
			}

			void Disconnect()
			{
				hI.Close();
				hI2.Close();
			}

			HRESULT Connect(const wchar_t* host, bool SSL = false, unsigned short Port = 0, DWORD flg = 0, const wchar_t* user = 0, const wchar_t* pass = 0)
			{
				if (_wcsicmp(Host.c_str(), host) != 0)
					Disconnect();
				Host = host;
				ssl = SSL;
				if (!hI)
					hI = InternetOpen(Agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
				if (!hI)
					return E_UNEXPECTED;
				if (!hI2)
				{
					if (SSL)
						hI2 = InternetConnect(hI, host, Port ? Port : INTERNET_DEFAULT_HTTPS_PORT, user, pass, INTERNET_SERVICE_HTTP, INTERNET_FLAG_SECURE | flg, 0);
					else
						hI2 = InternetConnect(hI, host, Port ? Port : INTERNET_DEFAULT_HTTP_PORT, user, pass, INTERNET_SERVICE_HTTP, flg, 0);
				}
				if (!hI2)
					return E_UNEXPECTED;
				InternetSetStatusCallback(hI2, 0);
				return S_OK;
			}

			ihandle RequestWithBuffer(const wchar_t* url, const wchar_t* verb = L"POST", std::initializer_list<wstring> hdrs = {}, const char* d = 0, size_t sz = 0, std::function<HRESULT(size_t sent, size_t tot, void*)> fx = nullptr, void* lp = 0, bool Once = true, DWORD ExtraSecurityFlags = 0)
			{
				memory_data_provider m(d, sz);
				return Request2(url, m, Once, verb, hdrs, fx, lp, 0, 0, 0, 0, ExtraSecurityFlags);
			}

			ihandle RequestWithFile(const wchar_t* url, const wchar_t* verb = L"POST", std::initializer_list<wstring> hdrs = {}, HANDLE hX = INVALID_HANDLE_VALUE, std::function<HRESULT(size_t sent, size_t tot, void*)> fx = nullptr, void* lp = 0, DWORD ExtraSecurityFlags = 0)
			{
				file_data_provider m(hX);
				return Request2(url, m, true, verb, hdrs, fx, lp, 0, 0, 0, 0, ExtraSecurityFlags);
			}

			ihandle Request2(const wchar_t* url, data_provider& dp, bool Once = true, const wchar_t* verb = L"POST", std::initializer_list<wstring> hdrs = {}, std::function<HRESULT(size_t sent, size_t tot, void*)> fx = nullptr, void* lp = 0, const char* extradata1 = 0, DWORD extradatasize1 = 0, const char* extradata2 = 0, DWORD extradatasize2 = 0, DWORD ExtraSecurityFlags = 0)
			{
				if (!url)
					return 0;
				wstring nurl;
				ihandle hI3;
				if (_wcsnicmp(url, L"http://", 7) == 0 || _wcsnicmp(url, L"https://", 8) == 0)
				{
					Disconnect();
					hI = InternetOpen(Agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
					if (_wcsicmp(verb, L"GET") == 0)
					{
						hI3 = InternetOpenUrl(hI, url, 0, 0, ssl ? INTERNET_FLAG_SECURE : 0, 0);
						return hI3;
					}
					else
					{
						URL_COMPONENTS u = { 0 };
						size_t ms = wcslen(url);
						u.dwStructSize = sizeof(u);

						unique_ptr<TCHAR> hn(new TCHAR[ms]);
						unique_ptr<TCHAR> un(new TCHAR[ms]);
						unique_ptr<TCHAR> pwd(new TCHAR[ms]);
						unique_ptr<TCHAR> dir(new TCHAR[ms]);
						unique_ptr<TCHAR> ex(new TCHAR[ms]);

						u.lpszHostName = hn.get();
						u.dwHostNameLength = (DWORD)ms;
						u.lpszUserName = un.get();
						u.dwUserNameLength = (DWORD)ms;
						u.lpszPassword = pwd.get();
						u.dwPasswordLength = (DWORD)ms;
						u.lpszUrlPath = dir.get();
						u.dwUrlPathLength = (DWORD)ms;
						u.lpszExtraInfo = ex.get();
						u.dwExtraInfoLength = (DWORD)ms;
						InternetCrackUrl(url, 0, 0, &u);

						nurl = u.lpszUrlPath;
						nurl += u.lpszExtraInfo;
						url = nurl.c_str();

						TCHAR* acct[] = { L"*/*",0 };
						if (_wcsnicmp(url, L"http://", 7) == 0)
							Connect(u.lpszHostName, false);
						else
							Connect(u.lpszHostName, true);

						hI3 = HttpOpenRequest(hI2, verb, url, 0, 0, (LPCTSTR*)acct, ssl ? INTERNET_FLAG_SECURE : 0, 0);
					}
				}
				else
				{
					TCHAR* acct[] = { L"*/*",0 };
					hI3 = HttpOpenRequest(hI2, verb, url, 0, 0, (LPCTSTR*)acct, ssl ? INTERNET_FLAG_SECURE : 0, 0);
				}

				if (!hI3)
					return 0;


				if (ExtraSecurityFlags)
				{
					DWORD dwFlags = 0;
					DWORD dwBuffLen = sizeof(dwFlags);
					InternetQueryOption(hI3, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)& dwFlags, &dwBuffLen);
					dwFlags |= ExtraSecurityFlags;
					InternetSetOption(hI3, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
				}


				for (size_t i = 0; i < hdrs.size(); i++)
				{
					auto h = (hdrs.begin() + i);
					if (h->length())
					{
						BOOL fx2 = HttpAddRequestHeaders(hI3, (LPCWSTR)h->c_str(), (DWORD)-1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
						if (!fx2)
							continue;
					}
				}

				if (((Once && dp.CanOnce()) || dp.s() == 0) && !extradata1 && !extradata2)
				{
					auto o = dp.Once();
					BOOL ldr = HttpSendRequest(hI3, 0, 0, (LPVOID)get<0>(o), (DWORD)get<1>(o));
					if (!ldr)
					{
						if (GetLastError() == 12044)
							ldr = HttpSendRequest(hI3, 0, 0, (LPVOID)get<0>(o), (DWORD)get<1>(o));
						if (!ldr)
							return 0;
					}

					if (fx)
						fx(get<1>(o), get<1>(o), lp);
					return hI3;
				}

				BOOL ldr = HttpSendRequestEx(hI3, 0, 0, 0, 0);
				if (!ldr)
					return 0;
				auto tot = dp.s();
				size_t Sent = 0;
				vector<char> b(4096);
				DWORD a = 0;
				if (extradata1)
					InternetWriteFile(hI3, extradata1, extradatasize1, &a);
				for (;;)
				{
					memset(b.data(), 0, 4096);
					auto ri = dp.Read(b.data(), 4096);
					if (!ri)
						break;
					if (!InternetWriteFile(hI3, b.data(), (DWORD)ri, &a))
					{
						DWORD dwe = 0;
						wchar_t rep[1000] = { 0 };
						DWORD dwb = 1000;
						InternetGetLastResponseInfo(&dwe, rep, &dwb);
						return 0;
					}
					if (a != ri)
						return 0;
					Sent += ri;
					if (fx)
					{
						auto rst = fx(Sent, tot, lp);
						if (FAILED(rst))
							return 0;
					}
				}
				if (extradata2)
					InternetWriteFile(hI3, extradata2, extradatasize2, &a);
				HttpEndRequest(hI3, 0, 0, 0);
				return hI3;
			}

			long Headers(ihandle & hI3, std::map<wstring, wstring> & t)
			{
				vector<wchar_t> lpOutBuffer(10000);
				DWORD dwSize = 10000;
				if (!HttpQueryInfo(hI3, HTTP_QUERY_RAW_HEADERS_CRLF,
					(LPVOID)lpOutBuffer.data(), &dwSize, NULL))
					return E_FAIL;
				vector<wstring> str;
				wstring s = lpOutBuffer.data();
				split(s, '\n', str);
				for (auto& a : str)
				{
					vector<wstring> str2;
					wcscpy_s(lpOutBuffer.data(), 10000, a.c_str());
					wchar_t* a1 = wcschr(lpOutBuffer.data(), ':');
					if (!a1)
						continue;
					*a1 = 0;
					a1++;
					if (wcslen(a1) && a1[wcslen(a1) - 1] == '\r')
						a1[wcslen(a1) - 1] = 0;
					while (wcslen(a1) && *a1 == ' ')
						a1++;
					t[lpOutBuffer.data()] = a1;
				}

				dwSize = 10000;
				if (!HttpQueryInfo(hI3, HTTP_QUERY_STATUS_CODE,
					(LPVOID)lpOutBuffer.data(), &dwSize, NULL))
					return E_FAIL;
				return _wtoi(lpOutBuffer.data());
			}

			HRESULT ReadToFile(ihandle & hI3, HANDLE hX, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = nullptr, void* lp = 0)
			{
				file_data_writer w(hX);
				return Read2(hI3, w, fx, lp);
			}

			HRESULT ReadToMemory(ihandle & hI3, vector<char> & m, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = nullptr, void* lp = 0)
			{
				memory_data_writer w;
				auto e = Read2(hI3, w, fx, lp);
				m = w.GetP();
				return e;
			}

			HRESULT Read2(ihandle & hI3, data_writer & dw, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = nullptr, void* lp = 0)
			{
				size_t Size = 0;
				unsigned long bfs = 10000;
				TCHAR ss[10000] = { 0 };
				if (!HttpQueryInfo(hI3, HTTP_QUERY_CONTENT_LENGTH, ss, &bfs, 0))
					Size = (size_t)-1;
				else
					Size = (size_t)_ttoi64(ss);
				BOOL ld = TRUE;

				unsigned long long TotalTransferred = 0;
				for (;;)
				{
					DWORD n;
					unique_ptr<char> Buff(new char[10100]);
					BOOL F = InternetReadFile(hI3, Buff.get(), 10000, &n);
					if (F == false && ld == TRUE)
					{
						ld = FALSE;
						InternetSetOption(hI, INTERNET_OPTION_HTTP_DECODING, (void*)& ld, sizeof(BOOL));
						F = InternetReadFile(hI3, Buff.get(), 10000, &n);
					}
					if (F == false)
						return E_FAIL;
					if (n == 0)
						break;

					TotalTransferred += n;

					if (dw.Write(Buff.get(), n) != n)
						return E_FAIL;
					if (fx)
					{
						auto rst = fx(TotalTransferred, Size, lp);
						if (FAILED(rst))
							return E_ABORT;
					}
				}
				return S_OK;
			}


			inline vector<char> datareturn(ihandle & r)
			{
				vector<char> out;
				ReadToMemory(r, out);
				return out;
			}

			inline string textreturn(ihandle & r)
			{
				vector<char> out;
				ReadToMemory(r, out);
				out.resize(out.size() + 1);
				char* p = (char*)out.data();
				return p;
			}

			HWND hAuthWindow = 0;
			inline void RunURL(const wchar_t* url)
			{
				const char* res = "\x01\x00\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\xC8\x08\xCF\x80\x00\x00\x00\x00\x00\x00\xFA\x01\x7E\x01\x00\x00\x00\x00\x00\x00\x08\x00\x90\x01\x00\x01\x4D\x00\x53\x00\x20\x00\x53\x00\x68\x00\x65\x00\x6C\x00\x6C\x00\x20\x00\x44\x00\x6C\x00\x67\x00\x00\x00";
				struct U
				{
					std::wstring url;
					REST* t;
				};
				U u;
				u.url = url;
				u.t = this;
				auto A_DP = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) -> INT_PTR
				{
					U* u = (U*)GetWindowLongPtr(hh, GWLP_USERDATA);
					HWND hX = GetDlgItem(hh, 888);
					switch (mm)
					{
					case WM_INITDIALOG:
					{
						SetWindowLongPtr(hh, GWLP_USERDATA, ll);
						u = (U*)GetWindowLongPtr(hh, GWLP_USERDATA);

						u->t->hAuthWindow = hh;
						hX = CreateWindowEx(0, L"AX", L"{8856F961-340A-11D0-A96B-00C04FD705A2}", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, hh, (HMENU)888, 0, 0);
						SendMessage(hh, WM_SIZE, 0, 0);
						SendMessage(hX, AX_INPLACE, 1, 0);
						ShowWindow(hh, SW_SHOWMAXIMIZED);

						CComPtr<IWebBrowser2> wb = 0;
						SendMessage(hX, AX_QUERYINTERFACE, (WPARAM)& IID_IWebBrowser2, (LPARAM)& wb);
						if (wb)
							wb->Navigate(_bstr_t(u->url.c_str()), 0, 0, 0, 0);
						return true;
					}
					case WM_CLOSE:
					{
						EndDialog(hh, 0);
						u->t->hAuthWindow = 0;
						return 0;
					}

					case WM_SIZE:
					{
						RECT rc;
						GetClientRect(hh, &rc);
						SetWindowPos(hX, 0, 0, 0, rc.right, rc.bottom - 0, SWP_SHOWWINDOW);
						return true;
					}
					}

					return 0;
				};

				std::thread tx([&]()
					{
						CoInitializeEx(0, COINIT_APARTMENTTHREADED);
						DialogBoxIndirectParam(GetModuleHandle(0), (LPCDLGTEMPLATEW)res, 0, A_DP, (LPARAM)& u);
					});
				tx.join();

				//		ShellExecute(0, L"open", url, 0, 0, SW_SHOWNORMAL);
			}



		};


		class OAUTH2 : public REST
		{
		private:
			std::string client;
			std::string secret;
			vector<std::string> scopes;
			std::string accesstoken;
			std::string refreshtoken;
			std::string code;
			std::wstring LoginEndpoint;
			std::wstring LogoutEndpoint;
			std::wstring RedirectURI;

		public:

			void SetClient(const char* c, const char* s, const wchar_t* reduri = L"", std::initializer_list<string> scops = {})
			{
				client = c;
				secret = s;
				RedirectURI = reduri;
				scopes = scops;
			}

			void SendEndpoints(const wchar_t* login = L"", const wchar_t* logout = L"")
			{
				LoginEndpoint = login;
				LogoutEndpoint = logout;
			}

			void Login()
			{

			}

		};

	}



	namespace GOD
	{
		template <int MR = 1>
		class TEVENT
		{
		public:
			HANDLE m = 0;
			TEVENT()
			{
				m = CreateEvent(0, MR, 0, 0);
			}
			void Close()
			{
				if (m)
					CloseHandle(m);
				m = 0;
			}
			DWORD Wait(DWORD i = INFINITE)
			{
				if (m)
				{
					return  WaitForSingleObject(m, i);
				}
				return WAIT_ABANDONED;
			}
			void Set()
			{
				if (m)
					SetEvent(m);
			}
			void Reset()
			{
				if (m)
					ResetEvent(m);
			}
			~TEVENT()
			{
				Close();
			}

			TEVENT(const TEVENT& m)
			{
				operator=(m);
			}
			TEVENT(TEVENT&& m)
			{
				operator=(std::forward<TEVENT<MR>>(m));
			}

			TEVENT& operator= (const TEVENT& b)
			{
				Close();
				DuplicateHandle(GetCurrentProcess(), b.m, GetCurrentProcess(), &m, 0, 0, DUPLICATE_SAME_ACCESS);
				return *this;
			}
			TEVENT& operator= (TEVENT&& b)
			{
				Close();
				m = b.m;
				b.m = 0;
				return *this;
			}


		};


		// astring class
		class astring : public std::string
		{
		public:
			astring& Format(const char* f, ...)
			{
				va_list args;
				va_start(args, f);

				int len = _vscprintf(f, args) + 100;
				if (len < 8192)
					len = 8192;
				std::vector<char> b(len);
				vsprintf_s(b.data(), len, f, args);
				assign(b.data());
				va_end(args);
				return *this;
			}

		};


		// ystring class, wstring <-> string wrapper
		class ystring : public std::wstring
		{
		private:
			mutable std::string asc_str_st;
		public:

			// Constructors
			ystring(HWND hh) : std::wstring()
			{
				AssignFromHWND(hh);
			}
			ystring(HWND hh, int ID) : std::wstring()
			{
				AssignFromHWND(GetDlgItem(hh, ID));
			}
			ystring::ystring() : std::wstring()
			{
			}
			ystring(const char* v, int CP = CP_UTF8)
			{
				EqChar(v, CP);
			}
			ystring(const std::string& v, int CP = CP_UTF8)
			{
				EqChar(v.c_str(), CP);
			}
			ystring(const wchar_t* f)
			{
				if (!f)
					return;
				assign(f);
			}
			ystring& Format(const wchar_t* f, ...)
			{
				va_list args;
				va_start(args, f);

				int len = _vscwprintf(f, args) + 100;
				if (len < 8192)
					len = 8192;
				std::vector<wchar_t> b(len);
				vswprintf_s(b.data(), len, f, args);
				assign(b.data());
				va_end(args);
				return *this;
			}

			// operator =
			void operator=(const char* v)
			{
				EqChar(v);
			}
			void operator=(const wchar_t* v)
			{
				assign(v);
			}
			void operator=(const std::wstring& v)
			{
				assign(v.c_str());
			}
			void operator=(const ystring& v)
			{
				assign(v.c_str());
			}
			void operator=(const std::string& v)
			{
				EqChar(v.c_str());
			}
			CLSID ToCLSID()
			{
				CLSID a;
				CLSIDFromString(c_str(), &a);
				return a;
			}
			void operator=(CLSID cid)
			{
				wchar_t ad[100] = { 0 };
				StringFromGUID2(cid, ad, 100);
				assign(ad);
			}

			operator const wchar_t* ()
			{
				return c_str();
			}

			// asc_str() and a_str() and operator const char*() 
			const std::string& asc_str(int CP = CP_UTF8) const
			{
				const wchar_t* s = c_str();
				int sz = WideCharToMultiByte(CP, 0, s, -1, 0, 0, 0, 0);
				std::vector<char> d(sz + 100);
				WideCharToMultiByte(CP, 0, s, -1, d.data(), sz + 100, 0, 0);
				asc_str_st = d.data();
				return asc_str_st;
			}
			operator const char* () const
			{
				return a_str();
			}
			const char* a_str(int CP = CP_UTF8) const
			{
				asc_str(CP);
				return asc_str_st.c_str();
			}

			long long ll() const
			{
				return atoll(a_str());
			}

			// Internal Convertor
			void EqChar(const char* v, int CP = CP_UTF8)
			{
				clear();
				if (!v)
					return;
				int sz = MultiByteToWideChar(CP, 0, v, -1, 0, 0);
				std::vector<wchar_t> d(sz + 100);
				MultiByteToWideChar(CP, 0, v, -1, d.data(), sz + 100);
				assign(d.data());
			}

			// From HWND
			void AssignFromHWND(HWND hh)
			{
				int zl = GetWindowTextLength(hh);
				std::vector<wchar_t> n(zl + 100);
				GetWindowTextW(hh, n.data(), zl + 100);
				assign(n.data());
			}
		};


		using namespace RESTAPI;
		class DRIVE : public RESTAPI::REST
		{
		protected:

			ystring cid;
			ystring secret;
			jsonxx::Object j;
			int port = 9932;
			TEVENT<> ev;
			XSOCKETN::XSOCKET x;
			vector<char> nd;

			ystring encode(ystring i)
			{
				vector<wchar_t> url2(1000);
				DWORD fi = 1000;
				InternetCanonicalizeUrl(i.c_str(), url2.data(), &fi, 0);
				return GOD::ystring(url2.data()).a_str();
			}

			void Acc()
			{
				vector<char> data(10000);
				for (;;)
				{
					auto a = x.Accept();
					XSOCKETN::XSOCKET su(a);
					memset(data.data(), 0, 10000);
					int ar = su.receive(data.data(), 10000, 0, 0);
					if (ar == 0 || ar == -1)
					{
						su.Close();
						break;
					}
					nd = data;
					char* ab = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nOK, We received an authorization code. \r\nYou may close this browser window.";
					su.transmit(ab, (int)strlen(ab), true, 0);
					su.Close();
					SendMessage(hAuthWindow, WM_CLOSE, 0xDEADFACE, 0);
				}
			}




		public:




			DRIVE(const char* ccid = 0, const char* ssid = 0, int HostPort = 9932)
			{
				SetClient(ccid, ssid, HostPort);
			}
			virtual void SetClient(const char* ccid = 0, const char* ssid = 0, int HostPort = 9932)
			{
				if (ccid)
					cid = ccid;
				if (ssid)
					secret = ssid;
				port = HostPort;
			}
			virtual string jsonreturn(ihandle& r)
			{
				vector<char> out;
				ReadToMemory(r, out);
				out.resize(out.size() + 1);
				char* p = (char*)out.data();
				return p;
			}

			virtual void Unauth() = 0;
			virtual int Auth(vector<string>&) = 0;
			virtual string CreateFolder(const char* fn, const char* pid) = 0;
			virtual string GetRootFolderID() = 0;
			virtual string IDFromPath(const char* Path, bool CreateIfNotExists = false) = 0;
			virtual string dir(const char* Path = 0, bool IsRDir = false, bool FoldersOnly = false) = 0;
			virtual string ItemProps(const char* id) = 0;
			virtual string SetProperty(const char* id, const char* n, const char* v) = 0;
			virtual HRESULT Download(const char* fid, HANDLE hF, vector<char> * arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0) = 0;
			virtual HRESULT Upload(bool Resumable, HANDLE hX, vector<char> * arr, const char* folderid, const char* filename, string & resumedata, string & returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0) = 0;
			virtual string Delete(const char* rp, const char* fid, bool Trash = false) = 0;
			virtual HRESULT HashItem(const char* id, string & Hash, ALG_ID & HashAlg) = 0;
		};


		void EnumNames(RGF::GOD::DRIVE& dd, std::string& j1, std::vector<std::tuple<std::string, std::string, std::string>>* all, int AT, int DirOnly)
		{
			using namespace std;
			using namespace RGF;
			jsonxx::Object jjj;
			jjj.parse(j1.c_str());
			jsonxx::Array j5 = jjj.get<jsonxx::Array>(AT == 3 ? "entries" : AT == 2 ? "value" : "items");
			//	MessageBoxA(0, j1.c_str(), 0, 0);
			for (unsigned int jj1 = 0; jj1 < j5.size(); jj1++)
			{
				auto el = j5.get<jsonxx::Object>(jj1);
				string jsi = dd.ItemProps(el.get<jsonxx::String>("id").c_str());
				if (jsi.length() == 0)
					continue;

				jsonxx::Object js;
				string titl;
				js.parse(jsi.c_str());

				if (AT == 2)
					js = el;

				if (AT == 3 || AT == 2)
					titl = js.get<jsonxx::String>("name");
				else
					titl = js.get<jsonxx::String>("title");

				// Remove & from title
				for (auto& cc : titl)
				{
					if (cc == '&')
						cc = '_';
				}

				auto id = js.get<jsonxx::String>("id");

				// ".tag": "folder"
				if (AT == 3)
				{
					bool Fold = false;
					if (js.get<jsonxx::String>(".tag") == string("folder"))
						Fold = true;
					if (DirOnly)
					{
						if (Fold)
						{
							auto tu = make_tuple<string, string, string>(move(id), move(titl), move("application/vnd.google-apps.folder"));
							if (all)
								all->push_back(tu);
						}
					}
					else
					{
						auto tu = make_tuple<string, string, string>(move(id), move(titl), move(Fold ? "application/vnd.google-apps.folder" : ""));
						if (all)
							all->push_back(tu);
					}
				}
				else
					if (AT == 2)
				{
					bool Fold = false;
					if (js.has<jsonxx::Object>("folder"))
						Fold = true;
					if (DirOnly)
					{
						if (Fold)
						{
							auto tu = make_tuple<string, string, string>(move(id), move(titl), move("application/vnd.google-apps.folder"));
							if (all)
								all->push_back(tu);
						}
					}
					else
					{
						auto tu = make_tuple<string, string, string>(move(id), move(titl), move(Fold ? "application/vnd.google-apps.folder" : ""));
						if (all)
							all->push_back(tu);
					}
				}
				else
				{
					auto mi = js.get<jsonxx::String>("mimeType");
					if (DirOnly)
					{
						if (mi == "application/vnd.google-apps.folder")
						{
							auto tu = make_tuple<string, string, string>(move(id), move(titl), move(mi));
							if (all)
								all->push_back(tu);
						}
					}
					else
					{
						auto tu = make_tuple<string, string, string>(move(id), move(titl), move(mi));
						if (all)
							all->push_back(tu);
					}
				}
			}

			std::sort(all->begin(), all->end(), [](const std::tuple<std::string, std::string, std::string>& i1, const std::tuple<std::string, std::string, std::string>& i2) -> bool
				{
					std::string mi1 = std::get<2>(i1);
					std::string mi2 = std::get<2>(i2);
					if (mi1 == "application/vnd.google-apps.folder" && mi2 != "application/vnd.google-apps.folder")
						return true;
					if (mi2 == "application/vnd.google-apps.folder" && mi1 != "application/vnd.google-apps.folder")
						return false;

					if (std::get<1>(i1) < std::get<1>(i2))
						return true;
					return false;
				});


		}




		class ONEDRIVE : public DRIVE
		{
		private:
			int port = 9932;
			ystring code;
			ystring access_token;
			ystring refresh_token;


			bool Login()
			{
				ystring url;
				url += L"https://login.microsoftonline.com/common/oauth2/v2.0/authorize?client_id=";
				url += cid;
//				url += L"&scope=wl.signin%20wl.offline_access%20wl.basic%20wl.skydrive%20wl.skydrive_update&response_type=code&redirect_uri=http://localhost:";
				url += L"&scope=Files.ReadWrite.AppFolder%20offline_access&response_type=code&redirect_uri=http://localhost:";
				ystring sport;
				sport.Format(L"%u", port);
				url += sport;


				x.Create();
				if (!x.BindAndListen(port))
					return false;
				std::thread t(&ONEDRIVE::Acc, this);
				t.detach();
				RunURL(url.c_str());

				// GET /?code=
				if (nd.size() == 0)
					return false;
				char* a1 = strstr(nd.data(), "GET /?code=");
				if (!a1)
					return false;
				a1 += 11;
				char* a2 = strchr(a1, ' ');
				if (!a2)
					return false;
				*a2 = 0;
				code = a1;
				return true;
			}

			bool GetTokens()
			{
				if (FAILED(Connect(L"login.microsoftonline.com", true, 443, INTERNET_FLAG_SECURE)))
					return false;
				vector<char>  d;
				wstring h1 = L"Content-type: application/x-www-form-urlencoded";
				ystring f = L"client_id=";
				f += cid;
				f += L"&grant_type=authorization_code";
				f += L"&code=";
				f += code;
				f += L"&redirect_uri=http://localhost:";
				ystring sport;
				sport.Format(L"%u", port);
				f += sport;
				f += L"&client_secret=";
				f += secret;
				string a = f.a_str();


				d.resize(a.length());
				memcpy(d.data(), a.data(), a.length());

				ev.Reset();
				auto hi = RequestWithBuffer(L"/common/oauth2/v2.0/token", L"POST", { h1 }, d.data(), (DWORD)d.size());
				vector<char> out;
				ReadToMemory(hi, out);
				out.resize(out.size() + 1);
				char* p = (char*)out.data();
				j.parse(p);
				if (j.has<jsonxx::String>("access_token"))
					access_token = j.get<jsonxx::String>("access_token");
				if (j.has<jsonxx::String>("refresh_token"))
					refresh_token = j.get<jsonxx::String>("refresh_token");
				x.Close();
				Disconnect();
				if (access_token.length() && refresh_token.length())
				{
					code.clear();
					return true;
				}
				return false;
			}

			bool GetAccessToken()
			{
				if (FAILED(Connect(L"login.microsoftonline.com", true, 443, INTERNET_FLAG_SECURE)))
					return false;
				vector<char>  d;
				wstring h1 = L"Content-type: application/x-www-form-urlencoded";
				ystring f = L"client_id=";
				f += cid;
				f += L"&redirect_uri=http://localhost:";
				ystring sport;
				sport.Format(L"%u", port);
				f += sport;
				f += L"&client_secret=";
				f += secret;
				f += L"&refresh_token=";
				f += refresh_token;
				f += L"&grant_type=refresh_token";
				string a = f.a_str();
				d.resize(a.length());
				memcpy(d.data(), a.data(), a.length());

				ev.Reset();
				auto hi = RequestWithBuffer(L"/common/oauth2/v2.0/token", L"POST", { h1 }, d.data(), (DWORD)d.size());
				vector<char> out;
				ReadToMemory(hi, out);
				out.resize(out.size() + 1);
				char* p = (char*)out.data();
				j.parse(p);
				if (j.has<jsonxx::String>("access_token"))
					access_token = j.get<jsonxx::String>("access_token");
				x.Close();
				Disconnect();
				if (access_token.length())
					return true;
				return false;
			}

		public:

			ONEDRIVE(const char* ccid = 0, const char* ssid = 0, int HostPort = 9932) : DRIVE(ccid, ssid, HostPort)
			{
			}

			virtual void Unauth()
			{
				access_token = L"";
				refresh_token = L"";
				wstring url = (L"https://login.microsoftonline.com/common/oauth2/v2.0/logout?post_logout_redirect_uri=");


				wstring rd = L"http://localhost:";
				ystring sport;
				sport.Format(L"%u", port);
				rd += sport;
				url += rd;
				RunURL(url.c_str());
			}

			virtual int Auth(vector<string> & toks)
			{
				toks.resize(3);
				access_token = toks[0];
				refresh_token = toks[1];
				code = toks[2];

				if (access_token.empty())
				{
					Login();
					GetTokens();
					toks[0] = access_token;
					toks[1] = refresh_token;
					toks[2] = code;
					if (access_token.empty())
						return 0;
					return 2;
				}
				// Test root
				string a = GetRootFolderID();
				if (a.empty())
				{
					code.clear();
					access_token.clear();
					if (!refresh_token.empty())
					{
						GetAccessToken();
					}
					if (access_token.empty())
					{
						refresh_token.clear();
						Login();
						GetTokens();
					}
					toks[0] = access_token;
					toks[1] = refresh_token;
					toks[2] = code;
					if (access_token.empty())
						return 0;
					return 2;
				}
				return 1;
			}


			virtual string CreateFolder(const char* fn, const char* pid)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
					return false;
				ystring re = "/v1.0/me/drive/items/";
				re += ystring(pid);
				re += L"/children";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				string dk;
				dk += "{\r\n\"name\": \"";
				dk += fn;
				dk += "\",\r\n\"folder\": { } \r\n }";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				return jsonreturn(hi);
			}

			virtual string GetRootFolderID()
			{
//				if (FAILED(Connect(L"127.0.0.1", FALSE, 7888, 0)))
				if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
					return false;
				ystring re = "/v1.0/me/drive/special/approot";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				auto hi = RequestWithBuffer(re.c_str(), L"GET", { h1, L"Host: graph.microsoft.com"});
				vector<char> out;
				ReadToMemory(hi, out);
				out.resize(out.size() + 1);
				char* a1 = (char*)out.data();
				j.parse(a1);
				if (j.has<jsonxx::String>("id")) return j.get<jsonxx::String>("id");
				return "";
			}

			virtual string dir(const char* Path, bool IsRDir = false, bool FoldersOnly = false)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true)))
					return "";
				string pr = IsRDir ? Path : IDFromPath(Path);
				if (pr.empty())
					return "";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				// GET https ://graph.microsoft.com/v5.0/folder.a6b2a7e8f2515e5e/files?access_token=ACCESS_TOKEN
				ystring re;
				re.Format(L"/v1.0/me/drive/items/%S/children", pr.c_str());
				auto hi = RequestWithBuffer(re.c_str(), L"GET",{ h1, L"Host: graph.microsoft.com"});
				return jsonreturn(hi);
			}

			virtual string IDFromPath(const char* Path, bool CreateIfNotExists = false)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true)))
					return "";
				auto s = GetRootFolderID();
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				if (!Path)
					return s;

				string pr = s;
				vector<char> P(10000);
				strcpy_s(P.data(), 10000, Path);
				char* a0 = P.data();
				bool Created = false;
				for (;;)
				{
					if (!a0 || !strlen(a0))
						return pr;

					char* a1 = strchr(a0, '\\');
					if (a1)
						* a1 = 0;



					ystring re;
					re.Format(L"/v1.0/me/drive/items/%S/children", pr.c_str());
					auto hi = RequestWithBuffer(re.c_str(), L"GET", { h1, L"Host: graph.microsoft.com" });
					auto oout = jsonreturn(hi);

					std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
					EnumNames(*this, oout, &AllItems, 2, 1);

					bool F = false;
					for (auto& it : AllItems)
					{
						if (_stricmp(a0, get<1>(it).c_str()) == 0)
						{
							pr = get<0>(it);
							F = true;
							break;
						}
					}


					if (!F)
					{
						if (!CreateIfNotExists || Created)
							return "";
						auto cid2 = CreateFolder(a0, pr.c_str());
						if (cid2.empty())
							return "";
						Created = true;
						continue;
					}


					a0 = a1;
					if (a1 != 0)
						a0++;
				}
			}



			virtual string SetProperty(const char* id, const char* n, const char* v)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true)))
					return "";
				ystring re;
				re.Format(L"/v5.0/%S", id);
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				wstring h2 = L"Content-Type: application/json";
				vector<char> dx;
				ystring p;
				p.Format(L"{\r\n\"%S\": \"%S\"\r\n}\r\n", n, v);
				auto hi = RequestWithBuffer(re.c_str(), L"PUT", { h1,h2 }, p.a_str(), (DWORD)strlen(p.a_str()));
				return jsonreturn(hi);
			}


			virtual string ItemProps(const char* id)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
					return "";
				ystring re;
				re.Format(L"/v5.0/%S?access_token=", id);
				re += access_token;
				auto hi = RequestWithBuffer(re.c_str(), L"GET");
				return jsonreturn(hi);
			}


			virtual HRESULT Download(const char* fid, HANDLE hF, vector<char> * arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
					return E_FAIL;

				// GET https://graph.microsoft.com/v5.0/file.a6b2a7e8f2515e5e.A6B2A7E8F2515E5E!126/content?access_token=ACCESS_TOKEN
				ystring re;
				re.Format(L"/v1.0/me/drive/items/%S/content", fid);
				ystring range;
				range.Format(L"Range: bytes=%llu-%llu", from, to);
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				auto hi = to == -1 ? RequestWithBuffer(re.c_str(), L"GET", {h1}) : RequestWithBuffer(re.c_str(), L"GET", { h1,range });
				HRESULT hr = 0;

				if (arr)
					hr = ReadToMemory(hi, *arr, fx, lp);
				else
					hr = ReadToFile(hi, hF, fx, lp);
				return hr;
			}

			virtual HRESULT Upload(bool Resumable, HANDLE hX, vector<char>* arr, const char* folderid, const char* filename, string& resumedata, string& returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx, void* lp)
			{
				return Upload2(Resumable, hX, arr->data(), arr->size(), folderid, filename, resumedata, returndata, fx, lp);
			}

			virtual HRESULT Upload2(bool Resumable, HANDLE hX, const char * arr,unsigned long long arrs, const char* folderid, const char* filename, string & resumedata, string & returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx, void* lp)
			{
				if (arrs > (2 * 1024 * 1024))
					return UploadLong(arr, arrs, folderid, filename, resumedata, returndata, fx, lp);
				UNREFERENCED_PARAMETER(resumedata);
				UNREFERENCED_PARAMETER(Resumable);
				if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
					return E_FAIL;
				ystring re;
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				re.Format(L"/v1.0/me/drive/items/%S:/%S:/content", folderid,filename);
				unsigned long long Size = 0;
				LARGE_INTEGER li;
				GetFileSizeEx(hX, &li);
				Size = li.QuadPart;
				ystring h3;
				h3.Format(L"Content-Length: %llu", Size);
				if (arr)
					h3.Format(L"Content-Length: %llu", arrs);
				auto hi = arr ? RequestWithBuffer(re.c_str(), L"PUT", { h1,h3 }, arr,(size_t) arrs, fx, lp) : RequestWithFile(re.c_str(), L"PUT", { h1,h3 }, hX, fx, lp);
				returndata = jsonreturn(hi);
				return S_OK;
			}
			virtual HRESULT UploadLong(const char* arr, unsigned long long arrs, const char* folderid, const char* filename, string& resumedata, string& returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx, void* lp)
			{
				try
				{
					if (FAILED(Connect(L"graph.microsoft.com", true, 443, INTERNET_FLAG_SECURE)))
						return E_FAIL;
					wstring h1 = L"Authorization: Bearer ";
					h1 += access_token;

					ystring re;

					re.Format(L"/v1.0/me/drive/items/%S:/%S:/createUploadSession", folderid,filename);

					ystring jsn;
					jsn.Format(L"{	\"item\": {	\"@odata.type\": \"microsoft.graph.driveItemUploadableProperties\",	\"@microsoft.graph.conflictBehavior\" : \"rename\", \"name\" : \"%S\" }}", filename);
					const char* aa = jsn.a_str();
					auto hi1 = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-type: application/json" }, aa, (size_t)strlen(aa));
					auto r1 = jsonreturn(hi1);
					jsonxx::Object o1;
					o1.parse(r1);
					if (!o1.has<jsonxx::String>("uploadUrl"))
						return E_FAIL;
					ystring uurl = o1.get<jsonxx::String>("uploadUrl");

					unsigned long long step = 327680;
					for (unsigned long long ii = 0; ii < arrs; ii += step)
					{
						auto mx = arrs - ii;
						if (mx > step)
							mx = step;

						ystring h2;
						h2.Format(L"Content-Length: %llu", mx);
						ystring h3;
						h3.Format(L"Content-Range: bytes %llu-%llu/%llu", ii,ii + mx - 1,arrs);

						auto hi2 = RequestWithBuffer(uurl.c_str(), L"PUT", { h1,h2,h3 }, arr + ii,(size_t) mx);
						auto r2 = jsonreturn(hi2);
						jsonxx::Object o2;
						o2.parse(r2);

						if (fx)
						{
							auto hr = fx(ii, arrs, lp);
							if (FAILED(hr))
							{
								// Destroy upload session
								RequestWithBuffer(uurl.c_str(), L"DELETE", { h1 }, 0,0);
								break;
							}
						}
					}

					return S_OK;

				}
				catch (...)
				{
					return E_FAIL;
				}

			}


			virtual string Delete(const char* rp, const char* fid, bool Trash = false)
			{
				UNREFERENCED_PARAMETER(rp);
				if (!Trash)
					return "";
				if (FAILED(Connect(L"graph.microsoft.com", true)))
					return "";
				ystring re;
				re.Format(L"/v5.0/%S?access_token=", fid);
				re += access_token;
				auto hi = RequestWithFile(re.c_str(), L"DELETE");
				return jsonreturn(hi);
			}

			virtual HRESULT HashItem(const char* id, string & Hash, ALG_ID & HashAlg)
			{
				UNREFERENCED_PARAMETER(id);
				UNREFERENCED_PARAMETER(Hash);
				UNREFERENCED_PARAMETER(HashAlg);
				return E_NOTIMPL;
			}


		};


		class GOOGLEDRIVE : public DRIVE
		{
		private:
			ystring auth_token;
			ystring access_token;
			ystring refresh_token;



			bool HasRefresh()
			{
				if (refresh_token.length() > 0)
					return true;
				return false;
			}
			bool HasAccess()
			{
				if (access_token.length() > 0)
					return true;
				return false;
			}

			ystring GetPermissionURL(const wchar_t* rd = 0)
			{
				ystring u = L"https://accounts.google.com/o/oauth2/auth?response_type=code&scope=https://www.googleapis.com/auth/drive.file&client_id=";
				//ystring u = L"https://accounts.google.com/o/oauth2/auth?response_type=code&scope=https://www.googleapis.com/auth/drive&client_id=";
				u += cid.c_str();
				if (rd)
				{
					u += L"&redirect_uri=";
					u += rd;
					u += L"&access_type=offline";
				}
				else
					u += L"&redirect_uri=urn:ietf:wg:oauth:2.0:oob&access_type=offline";
				return u;
			}


			int GetRefreshAndAccessToken()
			{
				if (auth_token.empty())
					return -1;
				Disconnect();
				if (FAILED(Connect(L"accounts.google.com", true)))
					return -1;

				ystring dk;

				// Build the thing
				dk = "code=";
				dk += auth_token;
				dk += L"&";
				dk += L"client_id=";
				dk += cid;
				dk += L"&";
				dk += L"client_secret=";
				dk += secret;
				dk += L"&";
				ystring rd;
				rd.Format(L"http://localhost:%u", port);
				dk += L"redirect_uri=";
				dk += rd;
				dk += L"&";
				dk += L"grant_type=authorization_code";
				string u = dk.a_str();

				wchar_t au[1000] = { 0 };
				swprintf_s(au, 1000, L"Content-Length: %u", (DWORD)u.length());
				auto hi = RequestWithBuffer(L"/o/oauth2/token", L"POST", { L"Content-Type: application/x-www-form-urlencoded",au }, u.data(), u.size());
				auto js = jsonreturn(hi);
				j.parse(js);
				if (j.has<jsonxx::String>("access_token"))
				{
					if (j.has<jsonxx::String>("refresh_token"))
					{
						access_token = j.get<jsonxx::String>("access_token");
						refresh_token = j.get<jsonxx::String>("refresh_token");
						auth_token.clear();
						return 1;
					}
				}
				return 0;
			}

			int GetAccessToken()
			{
				if (!HasRefresh())
					return -1;
				Disconnect();
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return -1;


				wchar_t au[1000] = { 0 };
				ystring dk;

				// Build the thing
				dk = L"refresh_token=";
				dk += refresh_token;
				dk += L"&";
				dk += L"client_id=";
				dk += cid;
				dk += L"&";
				dk += L"client_secret=";
				dk += secret;
				dk += L"&";
				dk += L"grant_type=refresh_token";
				string u = dk.a_str(CP_UTF8);
				swprintf_s(au, 1000, L"Content-Length: %u\r\n", (DWORD)u.length());
				auto hi = RequestWithBuffer(L"/oauth2/v3/token", L"POST", { L"Content-Type: application/x-www-form-urlencoded",au }, u.data(), u.size());
				auto js = jsonreturn(hi);
				j.parse(js);
				if (j.has<jsonxx::String>("access_token"))
				{
					access_token = j.get<jsonxx::String>("access_token");
					return 1;
				}
				return 0;
			}



		public:

			GOOGLEDRIVE(const char* ccid = 0, const char* ssid = 0, int HostPort = 9932) : DRIVE(ccid, ssid, HostPort)
			{
			}

			virtual void Unauth()
			{
				access_token = L"";
				refresh_token = L"";
			}
			virtual int Auth(vector<string>& toks)
			{
				toks.resize(3);
				access_token = toks[0];
				refresh_token = toks[1];
				auth_token = toks[2];

				if (!HasAccess() && !HasRefresh())
				{
					ystring rd;
					rd.Format(L"http://localhost:%u", port);
					auto purl = GetPermissionURL(rd.c_str());

					x.Create();
					if (!x.BindAndListen(port))
						return false;
					std::thread t(&GOOGLEDRIVE::Acc, this);
					t.detach();

					RunURL(purl.c_str());

					// GET /?code=
					if (nd.size() == 0)
						return false;

					char* a1 = strstr(nd.data(), "GET /?code=");
					if (!a1)
						return false;
					a1 += 11;
					char* a2 = strchr(a1, ' ');
					if (!a2)
						return false;
					*a2 = 0;
					auth_token = a1;

					GetRefreshAndAccessToken();
					toks[0] = access_token;
					toks[1] = refresh_token;
					toks[2] = auth_token;
					Disconnect();
					return 2;
				}
				if (!HasRefresh() && !HasAccess())
				{
					if (!GetRefreshAndAccessToken())
					{
						auth_token.clear();
						toks[0].clear();
						toks[1].clear();
						toks[2].clear();
						return Auth(toks);
					}
					toks[0] = access_token;
					toks[1] = refresh_token;
					toks[2] = auth_token;
					Disconnect();
					return 2;
				}
				else
					if (HasRefresh() && !HasAccess())
					{
						GetAccessToken();
						toks[0] = access_token;
						toks[1] = refresh_token;
						toks[2] = auth_token;
						Disconnect();
						return 2;
					}
				Disconnect();


				auto s = dir();
				j.parse(s.c_str());
				if (j.has<jsonxx::Object>("error"))
				{
					access_token.clear();
					int ac = GetAccessToken();
					if (ac == 0)
					{
						refresh_token.clear();
						toks[0].clear();
						toks[1].clear();
						toks[2].clear();
						return Auth(toks);
					}
					toks[0] = access_token;
					toks[1] = refresh_token;
					toks[2] = auth_token;
					return 2;
				}


				return 1;
			}

			virtual string CreateFolder(const char* fn, const char* pid)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return "";
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());
				ystring re;
				re.Format(L"/upload/drive/v2/files?uploadType=multipart");

				string dk;

				// Build the thing
				dk += "--34FB4BDF-BB60-4050-AF36-FD834F499ED3\r\nContent-Type: application/json; charset=UTF-8\r\n\r\n{\r\n\"title\": \"";
				dk += fn;
				if (pid && strcmp(pid, "root") != 0)
				{
					dk += "\",\r\n";
					dk += "\"parents\": [{\"id\":\"";
					dk += pid;
					dk += "\"}]";
				}
				else
					dk += "\"";
				dk += ",\r\n";
				dk += "\"mimeType\": \"application/vnd.google-apps.folder\"\r\n";
				dk += "\r\n";
				dk += "}\r\n\r\n";

				string dk2 = "\r\n--34FB4BDF-BB60-4050-AF36-FD834F499ED3--";
				unsigned long long sz = dk.length() + dk2.length();


				vector<wchar_t> au2(10000);
				swprintf_s(au2.data(), 10000, L"Content-Length: %llu\r\n", sz);

				vector<wchar_t> au3(10000);
				swprintf_s(au3.data(), 10000, L"Content-Type: multipart/related; boundary=34FB4BDF-BB60-4050-AF36-FD834F499ED3\r\n");

				ihandle hi;
				memory_data_provider m(0, 0);
				hi = Request2(re.c_str(), m, false, L"POST", { au.data(),au2.data(),au3.data() }, 0, 0, dk.data(), (DWORD)dk.size(), dk2.data(), (DWORD)dk2.size());
				return jsonreturn(hi);
			}

			virtual string GetRootFolderID()
			{
				return "root";
			}
			virtual string IDFromPath(const char* Path, bool CreateIfNotExists = false)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return "";
				auto s = GetRootFolderID();
				if (!Path)
					return s;

				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());

				string pr = s;
				vector<char> P(10000);
				strcpy_s(P.data(), 10000, Path);
				char* a0 = P.data();
				for (;;)
				{
					if (!a0 || !strlen(a0))
						return pr;

					char* a1 = strchr(a0, '\\');
					if (a1)
						* a1 = 0;

					ystring re;
					re.Format(L"/drive/v2/files/%S/children", pr.c_str());
					auto hi = RequestWithBuffer(re.c_str(), L"GET", { au.data() });
					vector<char> out;
					ReadToMemory(hi, out);
					out.resize(out.size() + 1);
					char* oout = out.data();
					j.parse(oout);
					if (!j.has<jsonxx::Array>("items"))
						return "";
					bool F = false;
					auto ja = j.get<jsonxx::Array>("items");
					for (size_t i = 0; i < ja.size(); i++)
					{
						auto& e = ja.get<jsonxx::Object>((unsigned int)i);
						if (!e.has<jsonxx::String>("id"))
							continue;
						auto id = e.get<jsonxx::String>("id");
						string fips = ItemProps(id.c_str());
						jsonxx::Object j2;
						j2.parse(fips.c_str());
						if (!j2.has<jsonxx::String>("title"))
							continue;
						ystring tit = j2.get<jsonxx::String>("title");
						if (_stricmp(tit.a_str(), a0) == 0)
						{
							pr = id;
							a0 = a1;
							if (a0)
								a0++;
							F = 1;
							break;
						}
					}
					if (!F)
					{
						if (!CreateIfNotExists)
							return "";
						auto cid2 = CreateFolder(a0, pr.c_str());
						if (cid2.empty())
							return "";
						jsonxx::Object j2;
						j2.parse(cid2.c_str());
						if (!j2.has<jsonxx::String>("id"))
							return "";
						pr = j2.get<jsonxx::String>("id");
						a0 = a1;
						if (a0)
							a0++;
						continue;
					}
				}
			}
			virtual string dir(const char* Path = 0, bool IsRDir = false,bool FoldersOnly = false)
			{
				string pr = IsRDir ? Path : IDFromPath(Path);
				if (pr.empty())
					return "";
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());

				ystring re;
				re.Format(L"/drive/v2/files/%S/children", pr.c_str());
				if (FoldersOnly)
					re.Format(L"/drive/v2/files/%S/children?q=mimeType%%3D'application/vnd.google-apps.folder'", pr.c_str());
				auto hi = RequestWithBuffer(re.c_str(), L"GET", { au.data() });
				return jsonreturn(hi);
			}

			virtual string ItemProps(const char* id)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return "";
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());
				ystring re;
				re.Format(L"/drive/v2/files/%S", id);
				auto hi = RequestWithBuffer(re.c_str(), L"GET", { au.data() });
				return jsonreturn(hi);
			}

			virtual string SetProperty(const char* id, const char* n, const char* v)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return "";
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());

				string dk; // json for the request
				dk += "{\r\n";
				dk += "\"visibility\" : \"public\", \r\n";
				dk += "\"key\" : \"";
				dk += n;
				dk += "\",\r\n";
				dk += "\"value\" : \"";
				dk += v;
				dk += "\"\r\n";
				dk += "}\r\n\r\n";


				vector<wchar_t> au2(10000);
				swprintf_s(au2.data(), 10000, L"Content-Length: %u", (DWORD)dk.length());

				vector<wchar_t> au3(10000);
				swprintf_s(au3.data(), 10000, L"Content-Type: application/json; charset=UTF-8");

				ystring re;
				re.Format(L"/drive/v2/files/%S/properties", id);
				auto hi = RequestWithBuffer(re.c_str(), L"POST", { au.data(),au2.data(),au3.data() }, dk.data(), dk.size());
				return jsonreturn(hi);
			}

			virtual HRESULT Download(const char* fid, HANDLE hF, vector<char>* arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return E_FAIL;
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());

				ystring re;
				re.Format(L"/drive/v2/files/%S?alt=media", fid);
				ystring range;
				range.Format(L"Range: bytes=%llu-%llu", from, to);
				auto hi = to == -1 ? RequestWithBuffer(re.c_str(), L"GET", { au.data() }) : RequestWithBuffer(re.c_str(), L"GET", { range,au.data() });
				HRESULT hr = 0;
				if (arr)
					hr = ReadToMemory(hi, *arr, fx, lp);
				else
					hr = ReadToFile(hi, hF, fx, lp);
				return hr;
			}


			HRESULT UploadOnce(HANDLE hX, const char* arr,size_t ars, const char* folderid, const char* filename, string& returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return E_FAIL;
				/*
				Disconnect();
				if (FAILED(Connect(L"192.168.10.5", false)))
				return "";
				*/
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());
				ystring re;
				re.Format(L"/upload/drive/v2/files?uploadType=multipart");

				string dk;


				unsigned long long fsz = 0;
				if (arr)
					fsz = ars;
				else
				{
					LARGE_INTEGER li;
					GetFileSizeEx(hX, &li);
					fsz = li.QuadPart;
				}


				// Build the thing
				dk += "--34FB4BDF-BB60-4050-AF36-FD834F499ED3\r\nContent-Type: application/json; charset=UTF-8\r\n\r\n{\r\n\"title\": \"";
				dk += filename;
				if (folderid)
				{
					dk += "\",\r\n";
					dk += "\"parents\": [{\"id\":\"";
					dk += folderid;
					dk += "\"}]";
				}
				else
					dk += "\"";
				dk += "\r\n";
				dk += "}\r\n\r\n";

				ystring r2;
				r2.Format(L"--34FB4BDF-BB60-4050-AF36-FD834F499ED3\r\nContent-Type: application/octet-stream\r\nContent-Length: %llu\r\n\r\n", fsz);
				dk += r2;
				string dk2 = "\r\n--34FB4BDF-BB60-4050-AF36-FD834F499ED3--";
				unsigned long long sz = dk.length() + dk2.length() + fsz;


				vector<wchar_t> au2(10000);
				swprintf_s(au2.data(), 10000, L"Content-Length: %llu\r\n", sz);

				vector<wchar_t> au3(10000);
				swprintf_s(au3.data(), 10000, L"Content-Type: multipart/related; boundary=34FB4BDF-BB60-4050-AF36-FD834F499ED3\r\n");

				ihandle hi;
				if (arr)
				{
					memory_data_provider m(arr, ars);
					hi = Request2(re.c_str(), m, false, L"POST", { au.data(),au2.data(),au3.data() }, fx, lp, dk.data(), (DWORD)dk.size(), dk2.data(), (DWORD)dk2.size());
				}
				else
				{
					file_data_provider m(hX);
					hi = Request2(re.c_str(), m, false, L"POST", { au.data(),au2.data(),au3.data() }, fx, lp, dk.data(), (DWORD)dk.size(), dk2.data(), (DWORD)dk2.size());
				}
				map<wstring, wstring> st;
				auto resp = Headers(hi, st);
				returndata = jsonreturn(hi);
				if (resp != 200)
					return E_FAIL;
				return S_OK;
			}

			HRESULT ContinueResumable(HANDLE hX, vector<char> * arr, const char* folderid, const char* filename, string & returndata, string & resumedata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return E_FAIL;
				unsigned long long fsz = 0;
				if (arr)
					fsz = arr->size();
				else
				{
					LARGE_INTEGER li;
					GetFileSizeEx(hX, &li);
					fsz = li.QuadPart;
				}

				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s", access_token.c_str());
				vector<wchar_t> au2(10000);
				swprintf_s(au2.data(), 10000, L"Content-Range: bytes */%llu", fsz);
				vector<wchar_t> au3(10000);
				swprintf_s(au3.data(), 10000, L"Content-Length: 0");

				auto hi = RequestWithBuffer(ystring(resumedata.c_str()).c_str(), L"PUT", { au.data(),au2.data(),au3.data() });
				map<wstring, wstring> hdrs;
				auto resp = Headers(hi, hdrs);
				auto range = ystring(hdrs[L"Range"].c_str());
				if (resp == 200)
					return S_OK; // File already uploaded
				if (resp != 308 && resp != 201)
					return UploadOnce(hX, arr ? arr->data() : 0 ,arr ? arr->size() : 0, folderid, filename, returndata, fx, lp);

				unsigned long long rss = 0;
				const wchar_t* tt = wcsrchr(range.c_str(), '-');
				if (tt)
				{
					tt++;
					rss = _ttoi64(tt);
					rss += 1;
				}
				swprintf_s(au2.data(), 10000, L"Content-Range: bytes %llu-%llu/%llu", rss, fsz - 1, fsz);
				swprintf_s(au3.data(), 10000, L"Content-Length: %llu\r\n", fsz - rss);
				hi.Close();


				if (rss == 0)
					au2[0] = 0;

				ystring re = resumedata;
				if (arr)
				{
					memory_data_provider m(arr->data() + rss, arr->size() - (size_t)rss);
					hi = Request2(re.c_str(), m, false, L"PUT", { au.data(),au2.data(),au3.data() }, fx, lp);
				}
				else
				{
					file_data_provider m(hX);
					LARGE_INTEGER li = { 0 };
					li.QuadPart = rss;
					SetFilePointerEx(hX, li, 0, FILE_CURRENT);
					hi = Request2(re.c_str(), m, false, L"PUT", { au.data(),au2.data(),au3.data() }, fx, lp);
				}
				map<wstring, wstring> st;
				resp = Headers(hi, st);
				returndata = jsonreturn(hi);
				if (resp != 200)
					return E_FAIL;
				return S_OK;
			}

			HRESULT BeginResumable(HANDLE hX, vector<char> * arr, const char* folderid, const char* filename, string & returndata, string & resumedata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return E_FAIL;
				/*
				Disconnect();
				if (FAILED(Connect(L"192.168.10.5", false)))
				return "";
				*/

				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());

				ystring re;


				if (resumedata.empty())
				{
					re.Format(L"/upload/drive/v2/files?uploadType=resumable");

					unsigned long long fsz = 0;
					if (arr)
						fsz = arr->size();
					else
					{
						LARGE_INTEGER li;
						GetFileSizeEx(hX, &li);
						fsz = li.QuadPart;
					}

					string dk;
					// Build the thing
					dk += "{\r\n\"title\": \"";
					dk += filename;
					if (folderid)
					{
						dk += "\",\r\n";
						dk += "\"parents\": [{\"id\":\"";
						dk += folderid;
						dk += "\"}]";
					}
					else
						dk += "\"";
					dk += "\r\n";
					dk += "}\r\n\r\n";

					vector<wchar_t> au2(10000);
					swprintf_s(au2.data(), 10000, L"Content-Length: %llu", (unsigned long long)dk.length());

					vector<wchar_t> au3(10000);
					swprintf_s(au3.data(), 10000, L"Content-Type: application/json; charset=UTF-8");

					vector<wchar_t> au4(10000);
					swprintf_s(au4.data(), 10000, L"X-Upload-Content-Length: %llu", fsz);

					vector<wchar_t> au5(10000);
					swprintf_s(au5.data(), 10000, L"X-Upload-Content-Type: application/octet-stream");


					ihandle hi = RequestWithBuffer(re.c_str(), L"POST", { au.data(),au2.data(),au3.data(),au4.data(),au5.data() }, dk.data(), (DWORD)dk.size(), fx, lp);
					map<wstring, wstring> hdrs;
					auto resp = Headers(hi, hdrs);
					if (resp != 200)
						return UploadOnce(hX, arr ? arr->data() : 0,arr ? arr->size() : 0, folderid, filename, returndata, fx, lp);
					resumedata = ystring(hdrs[L"Location"].c_str());
				}

				return ContinueResumable(hX, arr, folderid, filename, returndata, resumedata, fx, lp);
			}


			virtual HRESULT Upload(bool Resumable, HANDLE hX, vector<char> * arr, const char* folderid, const char* filename, string & resumedata, string & returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				if (Resumable)
					return BeginResumable(hX, arr, folderid, filename, returndata, resumedata, fx, lp);
				return UploadOnce(hX, arr ? arr->data() : 0, arr ? arr->size() : 0, folderid, filename, returndata, fx, lp);
			}


			virtual string Delete(const char* rp, const char* fid, bool Trash = false)
			{
				UNREFERENCED_PARAMETER(rp);
				if (FAILED(Connect(L"www.googleapis.com", true)))
					return "";
				vector<wchar_t> au(10000);
				swprintf_s(au.data(), 10000, L"Authorization: Bearer %s\r\n", access_token.c_str());
				ystring re;
				if (Trash)
				{
					re.Format(L"/drive/v2/files/%S/trash", fid);
					auto hi = RequestWithBuffer(re.c_str(), L"POST", { au.data() });
					return jsonreturn(hi);
				}
				else
				{
					re.Format(L"/drive/v2/files/%S", fid);
					auto hi = RequestWithBuffer(re.c_str(), L"DELETE", { au.data() });
					return jsonreturn(hi);
				}
			}

			virtual HRESULT HashItem(const char* f, string & Hash, ALG_ID & HashAlg)
			{
				if (!f)
					return E_FAIL;
				j.parse(f);
				if (!j.has<jsonxx::String>("md5Checksum"))
					return E_NOTIMPL;
				Hash = j.get<jsonxx::String>("md5Checksum");
				HashAlg = CALG_MD5;
				return S_OK;
			}


		};



		class DROPBOX : public DRIVE
		{
		private:
			int port = 9932;
			ystring access_token;
			ystring code;


			string GetCode()
			{
				ystring re;
				re.Format(L"https://www.dropbox.com/oauth2/authorize?response_type=code&client_id=%s&redirect_uri=http://localhost:%u", cid.c_str(), port);

				x.Create();
				if (!x.BindAndListen(port))
					return false;
				std::thread t(&DROPBOX::Acc, this);
				t.detach();
				ev.Reset();

				RunURL(re.c_str());
				ev.Wait(1000 * 30);
				
				// GET /?code=
				if (nd.size() == 0)
					return false;
				char* a1 = strstr(nd.data(), "GET /?code=");
				if (!a1)
					return "";
				a1 += 11;
				char* a2 = strchr(a1, ' ');
				if (!a2)
					return "";
				*a2 = 0;
				code = a1;
				return code.a_str();
			}


			bool GetAccessToken()
			{
				Disconnect();
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;


				string dk; // request
				dk += "code=";
				dk += code.a_str();
				dk += "&grant_type=authorization_code";
				dk += "&client_id=";
				dk += cid.a_str();
				dk += "&client_secret=";
				dk += secret.a_str();
				dk += "&redirect_uri=";
				ystring p;
				p.Format(L"http://localhost:%u", port);
				dk += p.a_str();

				ystring re;
				re.Format(L"/oauth2/token");
				auto hi = RequestWithBuffer(re.c_str(), L"POST", { L"Content-type: application/x-www-form-urlencoded" }, dk.data(), dk.size());
				auto jj = jsonreturn(hi);
				j.parse(jj.c_str());
				if (!j.has<jsonxx::String>("access_token"))
					return false;
				access_token = j.get<jsonxx::String>("access_token");
				Disconnect();
				return true;
			}



		public:

			DROPBOX(const char* ccid = 0, const char* ssid = 0, int HostPort = 9932) : DRIVE(ccid, ssid, HostPort)
			{
			}

			virtual void Unauth()
			{

			}


			virtual int Auth(vector<string>& toks)
			{
				toks.resize(3);
				access_token = toks[0];
				code = toks[2];
				if (access_token.empty())
				{
					if (code.empty())
					{
						GetCode();
						if (code.empty())
							return 0;
						toks[2] = code;
						GetAccessToken();
						if (access_token.empty())
							return 0;
						toks[0] = access_token;
					}
					return 2;
				}
				return 1;
			}

			virtual string CreateFolder(const char* fn, const char* pid)
			{
				UNREFERENCED_PARAMETER(pid);
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;
				ystring re = "/2/files/create_folder";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				string dk;
				dk += "{\r\n\"path\": \"";
				if (pid == 0 || strlen(pid) == 0)
					dk += "\\";
				dk += fn;
				dk += "\"\r\n}";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				return jsonreturn(hi);
			}

			virtual string GetRootFolderID()
			{
				return "";
			}
			virtual string IDFromPath(const char* Path, bool CreateIfNotExists = false)
			{
				auto s = GetRootFolderID();
				string tpath = Path;
				for (size_t i = 0; i < tpath.length(); i++)
				{
					if (tpath[i] == '\\') tpath[i] = '/';
				}

				// Get it
				string fx = s + tpath;
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;
				ystring re = "/2/files/get_metadata";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				string dk;
				dk += "{\r\n\"path\": \"";
				dk += fx;
				dk += "\"\r\n}";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				auto et = jsonreturn(hi);
				j.parse(et.c_str());
				if (j.has<jsonxx::String>("error_summary") && CreateIfNotExists)
				{
					et = CreateFolder(fx.c_str(), "");
					j.parse(et.c_str());
				}

				if (!j.has<jsonxx::String>("error_summary"))
				{
					return fx;// j.get<jsonxx::String>("id");
				}
				return "";
			}

			virtual string dir(const char* Path = 0, bool IsRDir = false, bool FoldersOnly = false)
			{
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;
				string pr = IsRDir ? Path : IDFromPath(Path);
				if (pr.empty() && strlen(Path) > 0)
					return "";

				ystring re = "/2/files/list_folder";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;

				string dk;
				dk += "{\r\n\"path\": \"";
				dk += pr.c_str();
				dk += "\"\r\n}";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				auto et = jsonreturn(hi);
				return et;
			}

			virtual string ItemProps(const char* id)
			{
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;
				ystring re = "/2/files/get_metadata";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				string dk;
				dk += "{\r\n\"path\": \"";
				dk += id;
				dk += "\"\r\n}";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				auto et = jsonreturn(hi);
				return et;
			}

			virtual string SetProperty(const char* id, const char* n, const char* v)
			{
				UNREFERENCED_PARAMETER(id);
				UNREFERENCED_PARAMETER(n);
				UNREFERENCED_PARAMETER(v);
				return "";
				/*			string ji = ItemProps(id);
							j.parse(ji.c_str());
							string teid = "ptid:R";

							ystring re = "/2/files/properties/add";
							wstring h1 = L"Authorization: Bearer ";
							h1 += access_token;
							string dk;
							dk += "{\r\n\"path\": \"";
							dk += id;
							dk += "\",\r\n";

							dk += " \"property_groups\": [\r\n{\r\n\"template_id\": \"";
							dk += teid;
							dk += "\",\r\n\"fields\" : [\r\n{\r\n \"name\": \"";
							dk += n;
							dk += "\",\r\n\"value\" : \"";
							dk += v;
							dk += "\"\r\n}\r\n	]\r\n}\r\n]\r\n";

							dk += "}";

							auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
							auto et = jsonreturn(hi);
							return et;
				*/
				return "";
			}

			virtual HRESULT Download(const char* fid, HANDLE hF, vector<char> * arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0)
			{
				if (FAILED(Connect(L"content.dropboxapi.com", true)))
					return false;

				ystring re = "/2/files/download";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				wstring h3 = L"Dropbox-API-Arg: {\"path\":\"";
				h3 += ystring(fid);
				h3 += L"\"}";
				ystring range;
				range.Format(L"Range: bytes=%llu-%llu", from, to);
				ihandle hi;
				if (to == -1)
				{
					std::initializer_list<wstring> hdrs = { h1, h3 };
					hi = RequestWithFile(re.c_str(), L"POST", hdrs);
				}
				else
				{
					std::initializer_list<wstring> hdrs = { h1, h3, range };
					hi = RequestWithFile(re.c_str(), L"POST", hdrs);
				}
				HRESULT hr = 0;
				if (arr)
					hr = ReadToMemory(hi, *arr, fx, lp);
				else
					hr = ReadToFile(hi, hF, fx, lp);

				return hr;
			}

			virtual HRESULT Upload2(bool Resumable, HANDLE hX, const char* arr, size_t arrs, const char* folderid, const char* filename, string& resumedata, string& returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				UNREFERENCED_PARAMETER(Resumable);
				UNREFERENCED_PARAMETER(resumedata);
				if (FAILED(Connect(L"content.dropboxapi.com", true)))
					return E_FAIL;

				ystring re = "/2/files/upload";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				wstring h2 = L"Content-Type: application/octet-stream";
				wstring h3 = L"Dropbox-API-Arg: {\"path\":\"";
				h3 += ystring(folderid);
				h3 += L"/";
				h3 += ystring(filename);
				h3 += L"\"}";
				ystring h4;
				unsigned long long sz = 0;
				if (arr)
					sz = arrs;
				else
				{
					LARGE_INTEGER li;
					GetFileSizeEx(hX, &li);
					sz = li.QuadPart;
				}
				h4.Format(L"Content-Length: %llu", sz);

				ihandle hi;
				if (arr)
					hi = RequestWithBuffer(re.c_str(), L"POST", { h1,h2,h3,h4 }, arr, arrs, fx, lp);
				else
					hi = RequestWithFile(re.c_str(), L"POST", { h1,h2,h3,h4 }, hX, fx, lp);
				auto et = jsonreturn(hi);
				j.parse(et.c_str());
				if (j.has<jsonxx::String>("error_summary"))
					return E_FAIL;
				returndata = et;
				return S_OK;

			}

			virtual HRESULT Upload(bool Resumable, HANDLE hX, vector<char> * arr, const char* folderid, const char* filename, string & resumedata, string & returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx = 0, void* lp = 0)
			{
				return Upload2(Resumable, hX, arr ? arr->data() : 0, arr ? arr->size() : 0, folderid, filename, resumedata, returndata, fx, lp);
			}

			virtual string Delete(const char* rp, const char* fid, bool Trash = false)
			{
				UNREFERENCED_PARAMETER(fid);
				if (!Trash)
					return false;
				if (FAILED(Connect(L"api.dropboxapi.com", true)))
					return false;
				ystring re = "/2/files/delete";
				wstring h1 = L"Authorization: Bearer ";
				h1 += access_token;
				string dk;
				dk += "{\r\n\"path\": \"";
				dk += rp;
				dk += "\"\r\n}";

				auto hi = RequestWithBuffer(re.c_str(), L"POST", { h1,L"Content-Type: application/json" }, dk.data(), dk.size());
				auto et = jsonreturn(hi);
				return et;
			}

			virtual HRESULT HashItem(const char* id, string & Hash, ALG_ID & HashAlg)
			{
				UNREFERENCED_PARAMETER(id);
				UNREFERENCED_PARAMETER(Hash);
				UNREFERENCED_PARAMETER(HashAlg);
				return E_NOTIMPL;
			}



		};


	}

	struct AUTH
	{
		std::string id;
		std::string secret;
		std::vector<std::string> tokens; // must be 3
		std::string root; // ID form
		std::string cd;

	};

	struct RGBF
	{
		int func = 0; // 0 save, 1 load
		HWND hParent = 0;
		bool NoBrowserKey = false;

		HRESULT rs = E_ABORT;
		GOD::ystring resultFile;
		GOD::ystring DefExt;
		std::vector<wchar_t> Filter;

		std::shared_ptr<RGF::GOD::GOOGLEDRIVE> goo = 0;
		std::shared_ptr<RGF::GOD::ONEDRIVE> one = 0;
		std::shared_ptr<RGF::GOD::DROPBOX> drop = 0;
		AUTH google;
		AUTH onedrive;
		AUTH db;

		std::wstring Title;

		// Saving
		DWORD sz = 0;
		const char* d = nullptr;
		std::function<HRESULT(char*, size_t)> cbf = nullptr;

		// Opening
		std::vector<char>* read = 0;


		// For Old style only
		HWND hH = 0;
		bool InProgress = false;
		bool ShouldCancelProp = false;

	};

	struct INSAVE
	{
		HWND hh = 0;
		bool ShouldCancel = false;
		RGBF* s;
	};



	struct LITEM
	{
		std::wstring tit;
		std::string id;
		bool RootBack = false;
	};


};


namespace RGF
{
	// RGF
	
	std::wstring SaveFileX = LR"(<StackPanel xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
        xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" Orientation="Vertical">
		<StackPanel Orientation="Vertical" x:Name="run" Visibility="Collapsed">
			<ProgressRing x:Name="runProgress" IsActive="true" Margin="10"  MinWidth="100" MinHeight="100"  Width="100" Height="100"/>
			<ProgressBar x:Name="runBar" Value="0" Maximum="100" Margin="10"/>
			<Button x:Name="runCancel" Margin="10">Cancel</Button>
		</StackPanel>



      <ContentDialog x:Name="goo_NF" IsPrimaryButtonEnabled="false"  >        <StackPanel>          <TextBlock FontWeight="Bold" Text="Enter folder name:" />          <TextBox Width="300" x:Name="gfname" Margin="0,10,0,10" />          <Border Background="LightBlue" BorderBrush="LightBlue" BorderThickness="2" Margin="0,10,0,10" />          <StackPanel Orientation="Horizontal" Margin="0,10,0,10" >            <Button Content="Create" x:Name="gf_OK" Margin="0,0,10,0"/>            <Button Content="Cancel" x:Name="gf_Cancel"/>          </StackPanel>        </StackPanel>      </ContentDialog>

		
<Pivot x:Name="pi">
    <PivotItem Header="Local">


		<StackPanel Orientation="Vertical" Margin="30">
			<StackPanel Orientation="Horizontal">
			<TextBox x:Name="localFile"
                     PlaceholderText="Filename"
                     Width="300" HorizontalAlignment="Left"/>
				<Button x:Name="localPick" Margin="10,0,0,0" Width="50">...</Button>
			</StackPanel>

			<Button x:Name="localSave" Margin="0,20,0,0">Save</Button>
		</StackPanel>



    </PivotItem>
    <PivotItem Header="Google Drive" x:Name="name_google">

		<StackPanel>
			<StackPanel Orientation="Vertical" Margin="30" x:Name="googleWaiting">
				<TextBlock Margin="20" Text="Please wait while loading folders..." />
				<ProgressRing IsActive="true" MinWidth="100" MinHeight="100"  />
			</StackPanel>
			<StackPanel Orientation="Horizontal" Margin="0,30,0,0" x:Name="googleDone" Visibility="Collapsed" >

				<StackPanel Orientation="Vertical" Margin="0,0,20,0">
						<AppBarButton Icon="Up" Label="Top" x:Name="googleTop" Width="150"/>						<AppBarButton Icon="NewFolder" Label="New folder" x:Name="googleNF" Width="150"/>						<AppBarButton Icon="Import" Label="Logout" x:Name="googleLogout" Width="150"/>				</StackPanel>


				<StackPanel Orientation="Vertical" >
					<TextBlock Margin="5" Text="Select a folder, then type a file name:" x:Name="googleHelp1"/>
					<StackPanel Orientation="Horizontal" >
					<TextBox x:Name="googleFile"
							 PlaceholderText="Filename"
							 Width="300" HorizontalAlignment="Left"/>
						<Button x:Name="googleSave" Margin="10,0,0,0" >Save to root</Button>
					</StackPanel>

					  <Grid>						<Grid.ColumnDefinitions>						  <ColumnDefinition Width="Auto" x:Name="ColDef1" />						</Grid.ColumnDefinitions>						<Grid.RowDefinitions>						  <RowDefinition Height="Auto" x:Name="RowDef1" />						</Grid.RowDefinitions>						<ListView Grid.Row="0"  Grid.Column="0" CanDragItems="True" x:Name="googleList" SelectionMode="Extended" ScrollViewer.VerticalScrollBarVisibility="Visible" />					  </Grid>				</StackPanel>

 


			</StackPanel>
		</StackPanel>

    </PivotItem>
    <PivotItem Header="OneDrive" x:Name="name_one">


		<StackPanel>
			<StackPanel Orientation="Vertical" Margin="30" x:Name="oneWaiting">
				<TextBlock Margin="20" Text="Please wait while loading folders..." />
				<ProgressRing IsActive="true" MinWidth="100" MinHeight="100"  />
			</StackPanel>
			<StackPanel Orientation="Horizontal" Margin="0,30,0,0" x:Name="oneDone" Visibility="Collapsed" >

				<StackPanel Orientation="Vertical" Margin="0,0,20,0">
						<AppBarButton Icon="Up" Label="Top" x:Name="oneTop" Width="150"/>						<AppBarButton Icon="NewFolder" Label="New folder" x:Name="oneNF" Width="150"/>						<AppBarButton Icon="Import" Label="Logout" x:Name="oneLogout" Width="150"/>				</StackPanel>


				<StackPanel Orientation="Vertical" >
					<TextBlock Margin="5" Text="Select a folder, then type a file name:" x:Name="oneHelp1"/>
					<StackPanel Orientation="Horizontal" >
					<TextBox x:Name="oneFile"
							 PlaceholderText="Filename"
							 Width="300" HorizontalAlignment="Left"/>
						<Button x:Name="oneSave" Margin="10,0,0,0" >Save to root</Button>
					</StackPanel>

					  <Grid>						<Grid.ColumnDefinitions>						  <ColumnDefinition Width="Auto" x:Name="ColDef1" />						</Grid.ColumnDefinitions>						<Grid.RowDefinitions>						  <RowDefinition Height="Auto" x:Name="RowDef1" />						</Grid.RowDefinitions>						<ListView Grid.Row="0"  Grid.Column="0" CanDragItems="True" x:Name="oneList" SelectionMode="Extended" ScrollViewer.VerticalScrollBarVisibility="Visible" />					  </Grid>				</StackPanel>

 


			</StackPanel>
		</StackPanel>


    </PivotItem>
    <PivotItem Header="Dropbox" x:Name="name_db">


		<StackPanel>
			<StackPanel Orientation="Vertical" Margin="30" x:Name="dbWaiting">
				<TextBlock Margin="20" Text="Please wait while loading folders..." />
				<ProgressRing IsActive="true" MinWidth="100" MinHeight="100"  />
			</StackPanel>
			<StackPanel Orientation="Horizontal" Margin="0,30,0,0" x:Name="dbDone" Visibility="Collapsed" >

				<StackPanel Orientation="Vertical" Margin="0,0,20,0">
						<AppBarButton Icon="Up" Label="Top" x:Name="dbTop" Width="150"/>						<AppBarButton Icon="NewFolder" Label="New folder" x:Name="dbNF" Width="150"/>						<AppBarButton Icon="Import" Label="Logout" x:Name="dbLogout" Width="150"/>				</StackPanel>


				<StackPanel Orientation="Vertical" >
					<TextBlock Margin="5" Text="Select a folder, then type a file name:" x:Name="dbHelp1"/>
					<StackPanel Orientation="Horizontal" >
					<TextBox x:Name="dbFile"
							 PlaceholderText="Filename"
							 Width="300" HorizontalAlignment="Left"/>
						<Button x:Name="dbSave" Margin="10,0,0,0" >Save to root</Button>
					</StackPanel>

					  <Grid>						<Grid.ColumnDefinitions>						  <ColumnDefinition Width="Auto" x:Name="ColDef1" />						</Grid.ColumnDefinitions>						<Grid.RowDefinitions>						  <RowDefinition Height="Auto" x:Name="RowDef1" />						</Grid.RowDefinitions>						<ListView Grid.Row="0"  Grid.Column="0" CanDragItems="True" x:Name="dbList" SelectionMode="Extended" ScrollViewer.VerticalScrollBarVisibility="Visible" />					  </Grid>				</StackPanel>

 


			</StackPanel>
		</StackPanel>


    </PivotItem>
</Pivot>
		</StackPanel>
)";



}





