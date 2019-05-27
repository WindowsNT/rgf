
#pragma comment(lib, "windowsapp")

namespace RGF
{
	// UWPLIB
	namespace UWPLIB
	{
		using namespace std;
		using namespace winrt::Windows::UI::Xaml::Hosting;
		using namespace winrt::Windows::Foundation::Collections;
		using namespace winrt::Windows::UI::Xaml::Markup;
		using namespace winrt::Windows::Foundation;
		using namespace winrt::Windows::UI;
		using namespace winrt::Windows::UI::Xaml;

#pragma pack(push,1)
		struct UWPCONTROL
		{
			DesktopWindowXamlSource xs;
			HWND hParent = 0;
			HWND hwnd = 0;
			HWND hwndDetailXamlIsland = 0;
			winrt::Windows::Foundation::IInspectable ins;
		};

		struct UWPNOTIFICATION
		{
			NMHDR n = { 0 };
			UWPCONTROL* s = 0;
		};
#pragma pack(pop)

#define UWPM_GET_CONTROL (WM_USER + 100)


#
		// ---

		inline map<wstring, UWPCONTROL*> controls;

		inline void GenericDestroy(HWND hh)
		{
			UWPCONTROL* s = (UWPCONTROL*)GetProp(hh, L"s");
			if (s)
			{
				auto str = s->ins.as<IFrameworkElement>().Name();
				controls.erase(str.c_str());
				RemoveProp(hh, L"s");
				delete s;
				s = 0;
			}
		}

		template <typename T>
		void GenericNotify(IInspectable const& sender, int Code)
		{
			auto rt = sender.as<T>();
			auto na = rt.Name();
			UWPCONTROL* s = 0;
			controls.readlock([&](const map<wstring, UWPCONTROL*>& m) {
				s = m.at(na.c_str());
				});
			if (!s)
				return; // Duh
			UWPNOTIFICATION n;
			n.n.hwndFrom = s->hwnd;
			n.n.code = Code;
			n.n.idFrom = GetDlgCtrlID(s->hwnd);
			n.s = (UWPCONTROL*)s;
			SendMessage(s->hParent, WM_NOTIFY, 0, (LPARAM)& n);

		}

		inline bool GetControlByName(const wchar_t* n, UWPCONTROL** ptr)
		{
			if (!n || !ptr)
				return false;
			UWPCONTROL* s = 0;
			s = controls.at(n);
			if (!s)
				return false;
			*ptr = s;
			return true;
		}




		inline UWPCONTROL* GenericCreate(HWND hP, HWND hh, LPVOID cs)
		{
			auto s = new UWPCONTROL;
			SetProp(hh, L"s", (HANDLE)s);
			s->hwnd = hh;
			s->hParent = hP;
			auto interopDetail = s->xs.as<IDesktopWindowXamlSourceNative>();
			auto hr = E_FAIL;

			// Generate the string if not there
			const wchar_t* strx = (const wchar_t*)cs;

			vector<wchar_t> tstr(10000);

			if (strx)
				hr = interopDetail->AttachToWindow(hh);
			if (!strx || FAILED(hr))
			{
				delete s;
				RemoveProp(hh, L"s");
				SetWindowLongPtr(hh, GWLP_USERDATA, 0);
				return 0;
			}
			try
			{
				interopDetail->get_WindowHandle(&s->hwndDetailXamlIsland);
				winrt::param::hstring str(strx);
				s->ins = XamlReader::Load(str);
				s->xs.Content(s->ins.as<UIElement>());
			}
			catch (...)
			{
				delete s;
				RemoveProp(hh, L"s");
				SetWindowLongPtr(hh, GWLP_USERDATA, 0);
				return 0;
			}

			auto str = s->ins.as<IFrameworkElement>().Name();
			controls[str.c_str()] = s;

			RECT rc = { 0 };
			GetClientRect(hh, &rc);
			MapWindowPoints(hh, hP, (LPPOINT)& rc, 2);
			//	ShowWindow(hh, SW_HIDE);
			//	SetWindowPos(s->hwnd, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
			SetWindowPos(s->hwndDetailXamlIsland, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);


			GetClientRect(hP, &rc);
			SetWindowPos(s->hwnd, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
			//SetWindowPos(s->hwndDetailXamlIsland, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
			return s;
		}

		inline ATOM Register_Custom()
		{
			auto WndProc = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) -> LRESULT
			{
				UWPCONTROL* s = (UWPCONTROL*)GetProp(hh, L"s");
				switch (mm)
				{
				case UWPM_GET_CONTROL:
				{
					return (LRESULT)s;
				}
				case WM_SETTEXT:
				{
					GenericDestroy(hh);
					HWND hP = GetParent(hh);
					s = GenericCreate(hP, hh, (LPVOID)ll);
					if (!s)
						return 0;
					return 1;
				}
				case WM_DESTROY:
				{
					GenericDestroy(hh);
					break;
				}
				case WM_CREATE:
				{
					return 0;
				}
				}
				return DefWindowProc(hh, mm, ww, ll);
			};

			WNDCLASSEXW wC = { 0 };
			wC.cbSize = sizeof(wC);
			wC.style = CS_GLOBALCLASS | CS_DBLCLKS;
			wC.lpfnWndProc = WndProc;
			wC.hInstance = GetModuleHandle(0);
			wC.lpszClassName = L"UWP_Custom";
			return RegisterClassExW(&wC);
		}

		inline void Register()
		{
			Register_Custom();
		}

	}


	struct AUTH
	{
		std::string id;
		std::string secret;
		std::vector<std::string> tokens; // must be 3
		std::string root; // ID form

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

	};

	RGF::UWPLIB::UWPCONTROL* u = 0;

	void EnumNames (RGF::GOD::DRIVE & dd, std::string & j1, std::vector<std::tuple<std::string, std::string,std::string>> * all, int AT, int DirOnly = 0)
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

	}

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

	void GoogleThreadLoad(INSAVE* s, GOD::ystring NewName, std::string NewID)
	{

		// We have data
		std::vector<std::tuple<std::string, std::string,std::string>> AllItems;
		std::string j = s->s->goo->dir(s->s->google.root.c_str(), true, s->s->func == 0 ? true : false);
		EnumNames(*s->s->goo, j, &AllItems, 1, !s->s->func);
		SendMessage(s->hh, WM_USER + 501, (WPARAM)& NewName, (LPARAM)& AllItems);
	};

	void OneThreadLoad(INSAVE * s, GOD::ystring NewName, std::string NewID)
	{
		// We have data
		std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
		std::string j = s->s->one->dir(s->s->onedrive.root.c_str(), true, true);
		EnumNames(*s->s->one, j, &AllItems, 2, !s->s->func);
		SendMessage(s->hh, WM_USER + 502, (WPARAM)& NewName, (LPARAM)& AllItems);
	}

#define TopView StackPanel

	using namespace winrt::Windows::UI;
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Controls;
	using namespace winrt::Windows::Foundation;


	INT_PTR CALLBACK A_DP(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
	{
		using namespace winrt;
		INSAVE* s = (INSAVE*)GetWindowLongPtr(hh, GWLP_USERDATA);
		HWND hX = GetDlgItem(hh, 901);
		switch (mm)
		{
		case WM_SIZE:
		{
			RECT rc;
			GetClientRect(hh, &rc);
			if (ww == SIZE_MINIMIZED)
			{
				ShowWindow(hh, SW_HIDE);
				return 0;
			}
			SetWindowPos(hX, 0, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);
			if (u)
			{
				SetWindowPos(u->hwndDetailXamlIsland, 0, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);
				GridLength v;
				v.Value = rc.bottom - 150;
				v.GridUnitType = GridUnitType::Pixel;
				GridLength v2;
				v2.Value = rc.right;
				v2.GridUnitType = GridUnitType::Pixel;
				if (v.Value > 0)
				{
					u->ins.as<TopView>().FindName(L"RowDef1").as<RowDefinition>().Height(v);
					u->ins.as<TopView>().FindName(L"ColDef1").as<ColumnDefinition>().Width(v2);
				}

			}
			return 0;
		}

		case WM_USER + 500:
		{
			auto Top = u->ins.as<TopView>();
			auto Prg = Top.FindName(L"runBar").as<ProgressBar>();
			//				INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
			Prg.Value(ww);
			return 0;
		}

		case WM_USER + 501:
		{
			// Fill Google Items
			// ll = &vector<tuple<string,string>>
			// ww = &string current root
			auto Top = u->ins.as<TopView>();
			Top.FindName(L"googleWaiting").as<StackPanel>().Visibility(Visibility::Collapsed);
			Top.FindName(L"googleDone").as<StackPanel>().Visibility(Visibility::Visible);

			if (s->s->func == 0)
			{
				GOD::ystring y;
				GOD::ystring curr = *(GOD::ystring*)ww;
				y.Format(L"Save to %s", curr.c_str());
				Top.FindName(L"googleSave").as<Button>().Content(winrt::box_value(y.c_str()));
			}

			auto lv = Top.FindName(L"googleList").as<ListView>();
			std::vector<std::tuple<std::string, std::string, std::string>>* vx = (std::vector<std::tuple<std::string, std::string, std::string>>*)ll;

			std::sort(vx->begin(), vx->end(), [](const std::tuple<std::string, std::string, std::string> & i1, const std::tuple<std::string, std::string, std::string> & i2) -> bool
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


			for (auto& a : *vx)
			{
				GOD::ystring y1 = std::get<1>(a);
				GOD::ystring y2 = std::get<0>(a);
				GOD::ystring sp;
				
				GOD::ystring mi = std::get<2>(a);
				if (mi == L"application/vnd.google-apps.folder")
					sp = GOD::ystring().Format(
					LR"(
						<StackPanel xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
								Orientation="Vertical" x:Name="foo">
							<TextBlock  Text="%s" x:Name="foo1" Foreground="Blue">
							</TextBlock>
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo2" />
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo3" />
						</StackPanel>

						)", (wchar_t*)y1.c_str(), (wchar_t*)y2.c_str(), (wchar_t*)mi.c_str());
				else
					sp = GOD::ystring().Format(
						LR"(
						<StackPanel xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
								Orientation="Vertical" x:Name="foo">
							<TextBlock  Text="%s" x:Name="foo1" Foreground="Black">
							</TextBlock>
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo2" />
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo3" />
						</StackPanel>

						)", (wchar_t*)y1.c_str(), (wchar_t*)y2.c_str(), (wchar_t*)mi.c_str());

				using namespace winrt::Windows::UI::Xaml::Markup;
				auto ins2 = XamlReader::Load(sp.c_str());

				lv.Items().Append(ins2);
			}

			SendMessage(hh, WM_SIZE, 0, 0);
			return 0;
		}

		case WM_USER + 502:
		{
			// Fill Sky Items
			// ll = &vector<tuple<string,string>>
			// ww = &string current root
			auto Top = u->ins.as<TopView>();
			Top.FindName(L"oneWaiting").as<StackPanel>().Visibility(Visibility::Collapsed);
			Top.FindName(L"oneDone").as<StackPanel>().Visibility(Visibility::Visible);

			if (s->s->func == 0)
			{
				GOD::ystring y;
				GOD::ystring curr = *(GOD::ystring*)ww;
				y.Format(L"Save to %s", curr.c_str());
				Top.FindName(L"oneSave").as<Button>().Content(winrt::box_value(y.c_str()));
			}

			auto lv = Top.FindName(L"oneList").as<ListView>();
			std::vector<std::tuple<std::string, std::string, std::string>>* vx = (std::vector<std::tuple<std::string, std::string, std::string>>*)ll;

			std::sort(vx->begin(), vx->end(), [](const std::tuple<std::string, std::string, std::string>& i1, const std::tuple<std::string, std::string, std::string>& i2) -> bool
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


			for (auto& a : *vx)
			{
				GOD::ystring y1 = std::get<1>(a);
				GOD::ystring y2 = std::get<0>(a);
				GOD::ystring sp;

				GOD::ystring mi = std::get<2>(a);
				if (mi == L"application/vnd.google-apps.folder")
					sp = GOD::ystring().Format(
						LR"(
						<StackPanel xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
								Orientation="Vertical" x:Name="foo">
							<TextBlock  Text="%s" x:Name="foo1" Foreground="Blue">
							</TextBlock>
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo2" />
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo3" />
						</StackPanel>

						)", (wchar_t*)y1.c_str(), (wchar_t*)y2.c_str(), (wchar_t*)mi.c_str());
				else
					sp = GOD::ystring().Format(
						LR"(
						<StackPanel xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
								Orientation="Vertical" x:Name="foo">
							<TextBlock  Text="%s" x:Name="foo1" Foreground="Black">
							</TextBlock>
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo2" />
							<TextBlock Visibility="Collapsed" Text="%s" x:Name="foo3" />
						</StackPanel>

						)", (wchar_t*)y1.c_str(), (wchar_t*)y2.c_str(), (wchar_t*)mi.c_str());

				using namespace winrt::Windows::UI::Xaml::Markup;
				auto ins2 = XamlReader::Load(sp.c_str());

				lv.Items().Append(ins2);
			}

			SendMessage(hh, WM_SIZE, 0, 0);
			return 0;
		}



		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hh, GWLP_USERDATA, ll);
			s = (INSAVE*)GetWindowLongPtr(hh, GWLP_USERDATA);
			s->hh = hh;
			if (s->s->Title.length())
				SetWindowText(hh, s->s->Title.c_str());
			else
			{
				if (s->s->func == 0)
					SetWindowText(hh, L"Save as...");
				else
					SetWindowText(hh, L"Open...");
			}

			SetWindowText(hX, SaveFileX.c_str());

			u = (RGF::UWPLIB::UWPCONTROL*)SendMessage(hX, (WM_USER + 100), 0, 0);
			auto Top = u->ins.as<TopView>();
			if (s->s->func == 1)
			{
				Top.FindName(L"googleHelp1").as<TextBlock>().Text(L"Select an item:");
				Top.FindName(L"googleFile").as<TextBox>().Visibility(Visibility::Collapsed);
				Top.FindName(L"googleSave").as<Button>().Visibility(Visibility::Collapsed);

			}
			Top.Tag(winrt::box_value<unsigned long long>((unsigned long long)s));
			// Local

			// Init
			auto localFile = Top.FindName(L"localFile").as<TextBox>();
			if (s->s->resultFile.length())
				localFile.Text(s->s->resultFile);

			// Pick
			auto localPick = Top.FindName(L"localPick").as<Button>();
			localPick.Click([](const IInspectable & ins, const RoutedEventArgs & r)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					OPENFILENAME of = { 0 };
					of.lpstrDefExt = s->s->DefExt.c_str();
					if (s->s->Filter.size() == 0)
						of.lpstrFilter = L"All files\0*.*\0\0";
					else
						of.lpstrFilter = s->s->Filter.data();
					of.lStructSize = sizeof(of);
					of.hwndOwner = s->hh;
					std::vector<wchar_t> fnx(10000);
					of.lpstrFile = fnx.data();
					if (s->s->resultFile.length())
						wcscpy_s(fnx.data(), 10000, s->s->resultFile.c_str());
					of.nMaxFile = 10000;
					if (s->s->func == 0)
						of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
					else
						of.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
					if (s->s->func == 0)
					{
						if (!GetSaveFileName(&of))
							return;
					}
					else
					{
						if (!GetOpenFileName(&of))
							return;
					}

					s->s->resultFile = fnx.data();
					auto localFile = Top.FindName(L"localFile").as<TextBox>();
					localFile.Text(fnx.data());
				});

			// Save
			auto localSave = Top.FindName(L"localSave").as<Button>();
			localSave.Click([](const IInspectable & ins, const RoutedEventArgs & r)
				{
					auto Top = u->ins.as<TopView>();

					auto localFile = Top.FindName(L"localFile").as<TextBox>();
					std::wstring fi = localFile.Text().c_str();
					if (fi.empty())
						return;

					// Hide Pivot
					Top.FindName(L"pi").as<Pivot>().Visibility(Visibility::Collapsed);
					Top.FindName(L"run").as<StackPanel>().Visibility(Visibility::Visible);

					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());

					if (s->s->func == 0)
					{
						HANDLE hX = CreateFile(fi.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
						if (hX == INVALID_HANDLE_VALUE)
						{
							s->s->rs = E_ACCESSDENIED;
							SendMessage(s->hh, WM_CLOSE, 0, 0);
							return;
						}

						s->s->rs = E_FAIL;
						if (s->s->d)
						{
							// Write once
							DWORD a = 0;
							WriteFile(hX, s->s->d, s->s->sz, &a, 0);
							if (a == s->s->sz)
								s->s->rs = S_OK;
						}

						CloseHandle(hX);
					}
					else
					{
						// Open
						if (s->s->read)
						{
							HANDLE hX = CreateFile(fi.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
							if (hX == INVALID_HANDLE_VALUE)
							{
								s->s->rs = E_ACCESSDENIED;
								SendMessage(s->hh, WM_CLOSE, 0, 0);
								return;
							}

							s->s->rs = E_FAIL;

							LARGE_INTEGER li;
							GetFileSizeEx(hX, &li);
							s->s->read->resize((size_t)li.QuadPart);
							// Read once
							DWORD a = 0;
							ReadFile(hX, s->s->read->data(), s->s->read->size(), &a, 0);
							if (a == s->s->read->size())
								s->s->rs = S_OK;

							CloseHandle(hX);
						}
						else
							s->s->rs = S_FALSE;

					}

					SendMessage(s->hh, WM_CLOSE, 0, 0);
					return;
				});

			auto piv = Top.FindName(L"pi").as<Pivot>();
			if (s->s->db.id.empty())
				piv.Items().RemoveAt(3);
			if (s->s->onedrive.id.empty())
				piv.Items().RemoveAt(2);
			if (s->s->google.id.empty())
				piv.Items().RemoveAt(1);
			// Pivot items
			piv.SelectionChanged([](const IInspectable & sender, const RoutedEventArgs&)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					int idx = Top.FindName(L"pi").as<Pivot>().SelectedIndex();
					if (idx == 2)
					{
						// OneDrive
				//		DebugBreak();
						if (!s->s->one)
						{
							auto ty1 = s->s->onedrive.id.c_str();
							auto ty2 = s->s->onedrive.secret.c_str();
							s->s->one = std::make_shared<RGF::GOD::ONEDRIVE>(ty1, ty2);
							s->s->onedrive.tokens.resize(3);
							int vrf = s->s->one->Auth(s->s->onedrive.tokens);
							if (vrf == 2)
								vrf = s->s->one->Auth(s->s->onedrive.tokens);
							s->s->onedrive.root = s->s->one->GetRootFolderID();


							auto lv = Top.FindName(L"oneList").as<ListView>();
							lv.IsDoubleTapEnabled(true);
							lv.DoubleTapped([](const IInspectable & sender, const RoutedEventArgs & drg)
								{
									auto Top = u->ins.as<TopView>();
									INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
									auto lv = sender.as<ListView>();
									auto sp = lv.SelectedItem().as<StackPanel>();
									auto pi = sp.FindName(L"foo1").as<TextBlock>();
									GOD::ystring fi = pi.Text().c_str();

									GOD::ystring mim = GOD::ystring(sp.FindName(L"foo3").as<TextBlock>().Text().c_str()).a_str();
									if (mim != L"application/vnd.google-apps.folder")
									{
										// Download this file

										// Hide Pivot
										Top.FindName(L"pi").as<Pivot>().Visibility(Visibility::Collapsed);
										Top.FindName(L"run").as<StackPanel>().Visibility(Visibility::Visible);

										//										virtual HRESULT Download(const char* fid, HANDLE hF, vector<char>* arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0)
																					//

										s->s->rs = S_OK;
										GOD::ystring fid = GOD::ystring(sp.FindName(L"foo2").as<TextBlock>().Text().c_str()).a_str();
										s->s->resultFile = fid;

										if (s->s->read)
										{
											auto foo = [](GOD::ystring fid, INSAVE* s)
											{
												auto hr = s->s->one->Download(fid.a_str(), 0, s->s->read, 0, (unsigned long long) - 1, [](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
													{
														INSAVE* s = (INSAVE*)lp;
														f *= 100;
														f = (int)(f / t);
														SendMessage(s->hh, WM_USER + 500,(WPARAM) f, 0);
														if (s->ShouldCancel)
															return E_FAIL;
														return S_OK;
													}, s);
												s->s->rs = hr;
												SendMessage(s->hh, WM_CLOSE, 0, 0);
											};

											std::thread tx(foo, fid, s);
											tx.detach();
											return;
										}
										SendMessage(s->hh, WM_CLOSE, 0, 0);
										return;

									}

									auto pr = s->s->onedrive.root;
									s->s->onedrive.root = GOD::ystring(sp.FindName(L"foo2").as<TextBlock>().Text().c_str()).a_str();
									Top.FindName(L"oneWaiting").as<StackPanel>().Visibility(Visibility::Visible);
									Top.FindName(L"oneDone").as<StackPanel>().Visibility(Visibility::Collapsed);
									lv.Items().Clear();

									std::thread t(OneThreadLoad, s, fi.asc_str(), s->s->onedrive.root);
									t.detach();

								});

							// Remove the ring
							if (vrf >= 1)
							{
								auto rid = s->s->one->GetRootFolderID();
								std::thread t(OneThreadLoad, s, "/", rid.c_str());
								t.detach();

							}
							else
								Top.FindName(L"oneWaiting").as<StackPanel>().Visibility(Visibility::Collapsed);
						}
					}
					if (idx == 1)
					{
						// Google
						if (!s->s->goo)
						{
							auto ty1 = s->s->google.id.c_str();
							auto ty2 = s->s->google.secret.c_str();
							s->s->goo = std::make_shared<RGF::GOD::GOOGLEDRIVE>(ty1, ty2);
							s->s->google.tokens.resize(3);
							int vrf = s->s->goo->Auth(s->s->google.tokens);
							if (vrf == 2)
								vrf = s->s->goo->Auth(s->s->google.tokens);
							s->s->google.root = s->s->goo->GetRootFolderID();


							auto lv = Top.FindName(L"googleList").as<ListView>();
							lv.IsDoubleTapEnabled(true);
							lv.DoubleTapped([](const IInspectable & sender, const RoutedEventArgs & drg)
								{
									auto Top = u->ins.as<TopView>();
									INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
									auto lv = sender.as<ListView>();
									auto sp = lv.SelectedItem().as<StackPanel>();
									auto pi = sp.FindName(L"foo1").as<TextBlock>();
									GOD::ystring fi = pi.Text().c_str();


									GOD::ystring mim = GOD::ystring(sp.FindName(L"foo3").as<TextBlock>().Text().c_str()).a_str();
									if (mim != L"application/vnd.google-apps.folder")
									{
									// Download this file

									// Hide Pivot
										Top.FindName(L"pi").as<Pivot>().Visibility(Visibility::Collapsed);
										Top.FindName(L"run").as<StackPanel>().Visibility(Visibility::Visible);

//										virtual HRESULT Download(const char* fid, HANDLE hF, vector<char>* arr, unsigned long long from = 0, unsigned long long to = (unsigned long long) - 1, std::function<HRESULT(unsigned long long, unsigned long long, void*)> fx = 0, void* lp = 0)
											//

										s->s->rs = S_OK;
										GOD::ystring fid = GOD::ystring(sp.FindName(L"foo2").as<TextBlock>().Text().c_str()).a_str();
										s->s->resultFile = fid;

										if (s->s->read)
										{
											auto foo = [](GOD::ystring fid,INSAVE* s)
											{
												auto hr = s->s->goo->Download(fid.a_str(), 0, s->s->read, 0, (unsigned long long) - 1, [](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
													{
														INSAVE* s = (INSAVE*)lp;
														f *= 100;
														f = (int)(f / t);
														SendMessage(s->hh, WM_USER + 500, (WPARAM)f, 0);
														if (s->ShouldCancel)
															return E_FAIL;
														return S_OK;
													}, s);
												s->s->rs = hr;
												SendMessage(s->hh, WM_CLOSE, 0, 0);
											};

											std::thread tx(foo,fid, s);
											tx.detach();
											return;
										}
										SendMessage(s->hh, WM_CLOSE, 0, 0);
										return;

									}
									auto pr = s->s->google.root;
									s->s->google.root = GOD::ystring(sp.FindName(L"foo2").as<TextBlock>().Text().c_str()).a_str();
									Top.FindName(L"googleWaiting").as<StackPanel>().Visibility(Visibility::Visible);
									Top.FindName(L"googleDone").as<StackPanel>().Visibility(Visibility::Collapsed);
									lv.Items().Clear();

									std::thread t(GoogleThreadLoad, s, fi, s->s->google.root);
									t.detach();

								});


							// Remove the ring
							if (vrf >= 1)
							{
								std::thread t(GoogleThreadLoad, s, "root", "root");
								t.detach();

							}
							else
							{
								Top.FindName(L"googleWaiting").as<StackPanel>().Visibility(Visibility::Collapsed);
							}


						}
					}

				});

				Top.FindName(L"oneTop").as<AppBarButton>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
					{
						auto Top = u->ins.as<TopView>();

						Top.FindName(L"oneWaiting").as<StackPanel>().Visibility(Visibility::Visible);
						Top.FindName(L"oneDone").as<StackPanel>().Visibility(Visibility::Collapsed);

						auto lv = Top.FindName(L"oneList").as<ListView>();
						lv.Items().Clear();
						INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
						s->s->onedrive.root = s->s->one->GetRootFolderID();
						std::thread t(OneThreadLoad, s, "/", s->s->onedrive.root);
						t.detach();
					});


			Top.FindName(L"googleTop").as<AppBarButton>().Click([](const IInspectable & ins, const RoutedEventArgs & r)
				{
					auto Top = u->ins.as<TopView>();

					Top.FindName(L"googleWaiting").as<StackPanel>().Visibility(Visibility::Visible);
					Top.FindName(L"googleDone").as<StackPanel>().Visibility(Visibility::Collapsed);

					auto lv = Top.FindName(L"googleList").as<ListView>();
					lv.Items().Clear();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					s->s->google.root = s->s->goo->GetRootFolderID();
					std::thread t(GoogleThreadLoad, s, "root", "root");
					t.detach();
				});

			Top.FindName(L"googleLogout").as<AppBarButton>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					s->s->google.tokens[0] = "";
					s->s->google.tokens[1] = "";
					s->s->goo->Unauth();
					s->s->goo = nullptr;
					Top.FindName(L"pi").as<Pivot>().SelectedIndex(0);
				});

			Top.FindName(L"oneLogout").as<AppBarButton>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					s->s->onedrive.tokens[0] = "";
					s->s->onedrive.tokens[1] = "";
					s->s->one->Unauth();
					s->s->one = nullptr;
					Top.FindName(L"pi").as<Pivot>().SelectedIndex(0);
				});

			Top.FindName(L"googleNF").as<AppBarButton>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
				{
					auto Top = u->ins.as<TopView>();
					//INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());

					auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
					auto apo = dlg.ShowAsync();

					auto bok = dlg.FindName(L"gf_OK").as<Button>();
					bok.Click([](const IInspectable& ins, const RoutedEventArgs& r)
						{
							auto Top = u->ins.as<TopView>();
							GOD::ystring url = Top.FindName(L"gfname").as<TextBox>().Text().c_str();
							INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
							auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
							dlg.Hide();

							s->s->goo->CreateFolder(url.a_str(), s->s->google.root.c_str());

							Top.FindName(L"googleWaiting").as<StackPanel>().Visibility(Visibility::Visible);
							Top.FindName(L"googleDone").as<StackPanel>().Visibility(Visibility::Collapsed);
							auto lv = Top.FindName(L"googleList").as<ListView>();
							lv.Items().Clear();
							std::thread t(GoogleThreadLoad, s, url, s->s->google.root);
							t.detach();


						});
					auto bca = dlg.FindName(L"gf_Cancel").as<Button>();
					bca.Click([](const IInspectable& ins, const RoutedEventArgs& r)
						{
							auto Top = u->ins.as<TopView>();
							//INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
							auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
							dlg.Hide();
						});




				});

			Top.FindName(L"oneNF").as<AppBarButton>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
				{
					auto Top = u->ins.as<TopView>();
					//INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());

					auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
					auto apo = dlg.ShowAsync();

					auto bok = dlg.FindName(L"gf_OK").as<Button>();
					bok.Click([](const IInspectable& ins, const RoutedEventArgs& r)
						{
							auto Top = u->ins.as<TopView>();
							GOD::ystring url = Top.FindName(L"gfname").as<TextBox>().Text().c_str();
							INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
							auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
							dlg.Hide();

							auto jsx = s->s->one->CreateFolder(url.a_str(), s->s->onedrive.root.c_str());

							Top.FindName(L"oneWaiting").as<StackPanel>().Visibility(Visibility::Visible);
							Top.FindName(L"oneDone").as<StackPanel>().Visibility(Visibility::Collapsed);
							auto lv = Top.FindName(L"oneList").as<ListView>();
							lv.Items().Clear();
							std::thread t(OneThreadLoad, s, url, s->s->onedrive.root);
							t.detach();


						});
					auto bca = dlg.FindName(L"gf_Cancel").as<Button>();
					bca.Click([](const IInspectable& ins, const RoutedEventArgs& r)
						{
							auto Top = u->ins.as<TopView>();
							//INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
							auto dlg = Top.FindName(L"goo_NF").as<ContentDialog>();
							dlg.Hide();
						});




				});


			if (s->s->func == 0)
				Top.FindName(L"googleSave").as<Button>().Click([](const IInspectable & ins, const RoutedEventArgs & r)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					//s->s->GoogleRoot = s->s->goo->GetRootFolderID();
					auto localFile = Top.FindName(L"googleFile").as<TextBox>();
					GOD::ystring fi = localFile.Text().c_str();

					if (fi.empty())
						return;

					// Hide Pivot
					Top.FindName(L"pi").as<Pivot>().Visibility(Visibility::Collapsed);
					Top.FindName(L"run").as<StackPanel>().Visibility(Visibility::Visible);


					if (wcschr(fi.c_str(), L'.') == 0)
					{
						if (s->s->DefExt.length())
						{
							fi += L".";
							fi += s->s->DefExt;
						}
					}

					// We upload to google
					auto up = [](GOD::ystring fi, INSAVE * s)
					{
						//							std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)>
						std::string ret;
						auto hr = s->s->goo->UploadOnce(0, s->s->d, s->s->sz, s->s->google.root.c_str(), fi.a_str(), ret,
							[](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
							{
								INSAVE* s = (INSAVE*)lp;
								f *= 100;
								f = (int)(f / t);
								SendMessage(s->hh, WM_USER + 500, (WPARAM)f, 0);
								if (s->ShouldCancel)
									return E_FAIL;
								return S_OK;
							}
						, s);
						s->s->rs = hr;
						if (SUCCEEDED(hr))
							s->s->resultFile = ret;
						SendMessage(s->hh, WM_CLOSE, 0, 0);

					};

					std::thread t(up, fi, s);
					t.detach();
				});

			if (s->s->func == 0)
				Top.FindName(L"oneSave").as<Button>().Click([](const IInspectable& ins, const RoutedEventArgs& r)
					{
						auto Top = u->ins.as<TopView>();
						INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
						auto localFile = Top.FindName(L"oneFile").as<TextBox>();
						GOD::ystring fi = localFile.Text().c_str();

						if (fi.empty())
							return;

						// Hide Pivot
						Top.FindName(L"pi").as<Pivot>().Visibility(Visibility::Collapsed);
						Top.FindName(L"run").as<StackPanel>().Visibility(Visibility::Visible);


						if (wcschr(fi.c_str(), L'.') == 0)
						{
							if (s->s->DefExt.length())
							{
								fi += L".";
								fi += s->s->DefExt;
							}
						}

						// We upload to onedrive
						auto up = [](GOD::ystring fi, INSAVE* s)
						{
							//							std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)>
							std::string ret;

							///virtual HRESULT Upload(bool Resumable, HANDLE hX, vector<char>* arr, const char* folderid, const char* filename, string& resumedata, string& returndata, std::function<HRESULT(unsigned long long f, unsigned long long t, void* lp)> fx, void* lp)
								//
							std::string rd;

							auto hr = s->s->one->Upload2(0, 0,s->s->d, s->s->sz, s->s->onedrive.root.c_str(), fi.a_str(),rd, ret,
								[](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
								{
									INSAVE* s = (INSAVE*)lp;
									f *= 100;
									f = (int)(f / t);
									SendMessage(s->hh, WM_USER + 500, f, 0);
									if (s->ShouldCancel)
										return E_FAIL;
									return S_OK;
								}
							, s);
							s->s->rs = hr;
							if (SUCCEEDED(hr))
								s->s->resultFile = ret;
							SendMessage(s->hh, WM_CLOSE, 0, 0);

						};

						std::thread t(up, fi, s);
						t.detach();
					});




			Top.FindName(L"runCancel").as<Button>().Click([](const IInspectable & ins, const RoutedEventArgs & r)
				{
					auto Top = u->ins.as<TopView>();
					INSAVE* s = (INSAVE*)winrt::unbox_value<unsigned long long>(Top.Tag());
					s->ShouldCancel = true;
				});

			return 0;
		}
		case WM_CLOSE:
		{
			s->s->goo = nullptr;
			s->s->one = nullptr;
			s->s->drop = nullptr;
			SetWindowLongPtr(hh, GWLP_USERDATA, 0);
			RGF::UWPLIB::GenericDestroy(hX);
			EndDialog(hh, 0);
			return 0;
		}

		}
	return 0;
	}


	HRESULT FunctionX(RGBF & s)
	{
		RGF::AXLIBRARY::AXRegister();
		RGF::UWPLIB::Register();


		if (s.sz == 0 && s.func == 0)
		{
			s.rs = E_INVALIDARG;
			return E_INVALIDARG;
		}

		INSAVE is;
		is.s = &s;



		// Dialog
		const char* res = "\x01\x00\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\xC8\x08\xCF\x80\x01\x00\x00\x00\x00\x00\xFA\x01\x1A\x01\x00\x00\x00\x00\x00\x00\x08\x00\x90\x01\x00\x01\x4D\x00\x53\x00\x20\x00\x53\x00\x68\x00\x65\x00\x6C\x00\x6C\x00\x20\x00\x44\x00\x6C\x00\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\xF8\x01\x1A\x01\x85\x03\x00\x00\x55\x00\x57\x00\x50\x00\x5F\x00\x43\x00\x75\x00\x73\x00\x74\x00\x6F\x00\x6D\x00\x00\x00\x43\x00\x75\x00\x73\x00\x74\x00\x6F\x00\x6D\x00\x31\x00\x00\x00\x00\x00";



		if (s.NoBrowserKey)
		{
			DialogBoxIndirectParam(GetModuleHandle(0), (LPCDLGTEMPLATEW)res, s.hParent, A_DP, (LPARAM)& is);

		}
		else
		{
			// Browser Emulation
			std::vector<wchar_t> fn(1000);
			GetModuleFileName(0, fn.data(), 1000);
			PathStripPath(fn.data());
			RGF::RKEY k(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
			k[fn.data()] = 11001UL;
			DialogBoxIndirectParam(GetModuleHandle(0), (LPCDLGTEMPLATEW)res, s.hParent, A_DP, (LPARAM)& is);
			k[fn.data()].Delete();
		}
		return s.rs;
	}

	HRESULT Save(RGBF & s)
	{
		s.func = 0;
		return FunctionX(s);
	}

	HRESULT Open(RGBF & s)
	{
		s.func = 1;
		return FunctionX(s);
	}


}


// 