// rgf

namespace RGF
{

	INT_PTR CALLBACK A_DP(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
	{
		return 0;
	}

	auto PrjWiz1DP = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) ->INT_PTR
	{
		PROPSHEETPAGE* p = (PROPSHEETPAGE*)GetWindowLongPtr(hh,GWLP_USERDATA);
		RGF::RGBF* r = p ? (RGF::RGBF*)p->lParam : 0;
		switch (mm)
		{
		case WM_CLOSE:
			{
			PropSheet_PressButton(GetParent(hh), PSBTN_CANCEL);
			return 0;
			}
			case WM_INITDIALOG:
			{
				SetWindowLongPtr(hh, GWLP_USERDATA, ll);
				p = (PROPSHEETPAGE*)ll;
				r = (RGF::RGBF*)p->lParam;

				if (r->resultFile.length())
					SetDlgItemText(hh, 101, r->resultFile.c_str());

				if (r->func == 1)
					SetDlgItemText(hh, 202, L"Open");

				return true;
			}
			case WM_COMMAND:
			{
				int LW = LOWORD(ww);
				if (LW == 201)
				{
					OPENFILENAME of = { 0 };
					of.lpstrDefExt = r->DefExt.c_str();
					if (r->Filter.size() == 0)
						of.lpstrFilter = L"All files\0*.*\0\0";
					else
						of.lpstrFilter = r->Filter.data();
					of.lStructSize = sizeof(of);
					of.hwndOwner = hh;
					std::vector<wchar_t> fnx(10000);
					of.lpstrFile = fnx.data();
					if (r->resultFile.length())
						wcscpy_s(fnx.data(), 10000, r->resultFile.c_str());
					of.nMaxFile = 10000;
					if (r->func == 0)
						of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
					else
						of.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
					if (r->func == 0)
					{
						if (!GetSaveFileName(&of))
							return 0;
					}
					else
					{
						if (!GetOpenFileName(&of))
							return 0;
					}

					r->resultFile = fnx.data();
					SetDlgItemText(hh, 101, r->resultFile.c_str());

				}

				if (LW == 202)
				{
					std::vector<wchar_t> t(1000);
					GetDlgItemText(hh, 101, t.data(), 1000);
					std::wstring fi = t.data();
					if (fi.empty())
						return 0;

					if (r->func == 0)
					{
						HANDLE hX = CreateFile(fi.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
						if (hX == INVALID_HANDLE_VALUE)
						{
							r->rs = E_ACCESSDENIED;
							SendMessage(hh, WM_CLOSE, 0, 0);
							return 0;
						}

						r->rs = E_FAIL;
						if (r->d)
						{
							// Write once
							DWORD a = 0;
							WriteFile(hX, r->d, r->sz, &a, 0);
							if (a == r->sz)
								r->rs = S_OK;
						}

						CloseHandle(hX);
					}
					else
					{
						// Open
						if (r->read)
						{
							HANDLE hX = CreateFile(fi.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
							if (hX == INVALID_HANDLE_VALUE)
							{
								r->rs = E_ACCESSDENIED;
								SendMessage(hh, WM_CLOSE, 0, 0);
								return 0;
							}

							r->rs = E_FAIL;

							LARGE_INTEGER li;
							GetFileSizeEx(hX, &li);
							r->read->resize((size_t)li.QuadPart);
							// Read once
							DWORD a = 0;
							ReadFile(hX, r->read->data(), (DWORD)r->read->size(), &a, 0);
							if (a == r->read->size())
								r->rs = S_OK;

							CloseHandle(hX);
						}
						else
							r->rs = S_FALSE;

					}

					SendMessage(hh, WM_CLOSE, 0, 0);
				}

				return 0;
			}
		}
		return 0;
	};

	void ListInsertColumn(HWND hL, int i, UINT mask, int fmt, int cx, TCHAR* txt, int txtm, int sub, int iI, int iO)
	{
		LV_COLUMN lc = { mask,fmt,cx,txt,txtm,sub,iI,iO };
		ListView_InsertColumn(hL, i, &lc);
	}

	void AutoSizeLVColumn(HWND hL, int j)
	{
		SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE);
		int a1 = ListView_GetColumnWidth(hL, j);
		SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE_USEHEADER);
		int a2 = ListView_GetColumnWidth(hL, j);
		if (a1 > a2)
			SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE);
	}


	void GoogleThreadLoad(RGBF* r, GOD::ystring NewName, std::string NewID,HWND hh)
	{
		// We have data
		std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
		std::string j = r->goo->dir(r->google.root.c_str(), true, r->func == 0 ? true : false);
		RGF::GOD::EnumNames(*r->goo, j, &AllItems, 1, !r->func);
		SendMessage(hh, WM_USER + 501, (WPARAM)& NewName, (LPARAM)& AllItems);
	};

	auto PrjWiz2DP = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) ->INT_PTR
	{
		PROPSHEETPAGE* p = (PROPSHEETPAGE*)GetWindowLongPtr(hh, GWLP_USERDATA);
		RGF::RGBF* r = p ? (RGF::RGBF*)p->lParam : 0;
		HWND hL = GetDlgItem(hh, 901);
		switch (mm)
		{
		case WM_CLOSE:
		{
			PropSheet_PressButton(GetParent(hh), PSBTN_CANCEL);
			return 0;
		}
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hh, GWLP_USERDATA, ll);
			p = (PROPSHEETPAGE*)ll;
			r = (RGF::RGBF*)p->lParam;
			SendDlgItemMessage(hh, 801, PBM_SETMARQUEE, true, 0);
			DWORD LEST = LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
			ListView_SetExtendedListViewStyle(hL, LEST);
			ListInsertColumn(hL, 0, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 30, L"Name", 0, 0, 0, 0);
			ListInsertColumn(hL, 1, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 0, L"ID", 0, 0, 0, 0);
			ListInsertColumn(hL, 2, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 0, L"MIME", 0, 0, 0, 0);

			ShowWindow(GetDlgItem(hh, 701), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 801), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 901), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 101), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 202), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 211), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 212), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 213), SW_HIDE);

			auto ty1 = r->google.id.c_str();
			auto ty2 = r->google.secret.c_str();
			r->goo = std::make_shared<RGF::GOD::GOOGLEDRIVE>(ty1, ty2);
			r->google.tokens.resize(3);
			int vrf = r->goo->Auth(r->google.tokens);
			if (vrf == 2)
				vrf = r->goo->Auth(r->google.tokens);
			r->google.root = r->goo->GetRootFolderID();
			if (vrf >= 1)
			{
				std::thread t(GoogleThreadLoad, r, "root", "root",hh);
				t.detach();
			}
			else
			{
				ShowWindow(GetDlgItem(hh, 701), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 801), SW_HIDE);
			}


			return true;
		}
		case WM_NOTIFY:
		{
			NMHDR* n = (NMHDR*)ll;
			if (n->hwndFrom == hL && n->code == NM_DBLCLK)
			{
				int L = ListView_GetNextItem(hL, -1, LVIS_SELECTED);
				if (L == -1)
					return 0;

				std::vector<wchar_t> t1(1000);
				std::vector<wchar_t> t2(1000);
				std::vector<wchar_t> t3(1000);
				ListView_GetItemText(hL, L, 0, t1.data(), 1000);
				ListView_GetItemText(hL, L, 1, t2.data(), 1000);
				ListView_GetItemText(hL, L, 2, t3.data(), 1000);
				if (_wcsicmp(t3.data(), L"application/vnd.google-apps.folder") == 0)
				{
					// Enter 
					auto pr = r->onedrive.root;
					r->google.cd += GOD::ystring(t1.data()).a_str();
					r->google.cd += "\\";
					r->google.root = GOD::ystring(t2.data()).a_str();
					ShowWindow(GetDlgItem(hh, 701), SW_SHOW);
					ShowWindow(GetDlgItem(hh, 801), SW_SHOW);
					ShowWindow(GetDlgItem(hh, 901), SW_HIDE);
					ShowWindow(GetDlgItem(hh, 101), SW_HIDE);
					ShowWindow(GetDlgItem(hh, 202), SW_HIDE);
					ShowWindow(GetDlgItem(hh, 211), SW_HIDE);
					ShowWindow(GetDlgItem(hh, 212), SW_HIDE);
					ShowWindow(GetDlgItem(hh, 213), SW_HIDE);
					ListView_DeleteAllItems(hL);

					GOD::ystring NewName = GOD::ystring(t1.data()).a_str();
					std::string NewID = GOD::ystring(t2.data()).a_str();
					std::thread t(GoogleThreadLoad, r, NewName, NewID,hh);
					t.detach();

				}
			}

			return 0;
		}
		case WM_COMMAND:
		{
			int LW = LOWORD(ww);
			if (LW == 211)
			{
				// Top
				ShowWindow(GetDlgItem(hh, 701), SW_SHOW);
				ShowWindow(GetDlgItem(hh, 801), SW_SHOW);
				ShowWindow(GetDlgItem(hh, 901), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 101), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 202), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 211), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 212), SW_HIDE);
				ShowWindow(GetDlgItem(hh, 213), SW_HIDE);
				r->google.root = r->goo->GetRootFolderID();
				std::thread t(GoogleThreadLoad, r, "root", "root",hh);
				t.detach();
			}
			return 0;
		}


		case WM_USER + 501:
		{
			// Fill Google Items
			// ll = &vector<tuple<string,string>>
			// ww = &string current root
			ShowWindow(GetDlgItem(hh, 701), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 801), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 901), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 101), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 202), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 211), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 212), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 213), SW_SHOW);

			if (r->func == 0)
			{
				GOD::ystring y;
				GOD::ystring curr = *(GOD::ystring*)ww;
				y.Format(L"Save to %s", curr.c_str());
				SetDlgItemText(hh, 202, y.c_str());
			}

			std::vector<std::tuple<std::string, std::string, std::string>>* vx = (std::vector<std::tuple<std::string, std::string, std::string>>*)ll;


			for (auto& a : *vx)
			{
				GOD::ystring y1 = std::get<1>(a);
				GOD::ystring y2 = std::get<0>(a);
				GOD::ystring sp;

				std::vector<wchar_t> tt(1000);
				GOD::ystring mi = std::get<2>(a);
				if (mi == L"application/vnd.google-apps.folder")
				{

					_stprintf_s(tt.data(), 1000, _T("%s"), y1.c_str());
					LV_ITEM lv = { 0 };
					lv.mask = LVIF_TEXT | LVIF_PARAM;
					lv.iItem = ListView_GetItemCount(hL);
					lv.pszText = tt.data();
					lv.lParam = (LPARAM)lv.iItem;
					int L = ListView_InsertItem(hL, &lv);

					ListView_SetItemText(hL, L, 1,(LPWSTR) y2.c_str());
					ListView_SetItemText(hL, L, 2,(LPWSTR)mi.c_str());

				}
				else
				{
				}

				AutoSizeLVColumn(hL, 0);
			}

			SendMessage(hh, WM_SIZE, 0, 0);
			return 0;
		}

		}
		return 0;
	};



	auto PrjWiz3DP = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) ->INT_PTR
	{
		PROPSHEETPAGE* p = (PROPSHEETPAGE*)GetWindowLongPtr(hh, GWLP_USERDATA);
		RGF::RGBF* r = p ? (RGF::RGBF*)p->lParam : 0;
		switch (mm)
		{
		case WM_CLOSE:
		{
			PropSheet_PressButton(GetParent(hh), PSBTN_CANCEL);
			return 0;
		}
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hh, GWLP_USERDATA, ll);
			p = (PROPSHEETPAGE*)ll;
			r = (RGF::RGBF*)p->lParam;


			return true;
		}
		case WM_COMMAND:
		{
			int LW = LOWORD(ww);
			return 0;
		}
		}
		return 0;
	};



	HRESULT FunctionX(RGBF& s)
	{
		RGF::AXLIBRARY::AXRegister();


		if (s.sz == 0 && s.func == 0)
		{
			s.rs = E_INVALIDARG;
			return E_INVALIDARG;
		}

		INSAVE is;
		is.s = &s;

		// Create the Sheet
		std::vector<PROPSHEETPAGE> Pages;


		// Local
		if (true)
		{
			PROPSHEETPAGE p1 = { 0 };
			p1.dwSize = sizeof(p1);
			p1.pszHeaderTitle = L"Local";
			p1.lParam = 0;
			p1.hInstance = GetModuleHandle(0);
			p1.pszTemplate = L"DIALOG_RGF_LOCAL";
			p1.pfnDlgProc = PrjWiz1DP;
			p1.lParam = (LPARAM)&s;
			Pages.push_back(p1);
		}
		// Google
		if (true)
		{
			PROPSHEETPAGE p1 = { 0 };
			p1.dwSize = sizeof(p1);
			p1.pszHeaderTitle = L"Google Drive";
			p1.lParam = 0;
			p1.hInstance = GetModuleHandle(0);
			p1.pszTemplate = L"DIALOG_RGF_GOOGLE";
			p1.pfnDlgProc = PrjWiz2DP;
			p1.lParam = (LPARAM)& s;
			Pages.push_back(p1);
		}

		PROPSHEETHEADER hdr = { 0 };
		hdr.dwSize = sizeof(hdr);
		hdr.dwFlags = PSH_PROPSHEETPAGE;
		hdr.hwndParent = s.hParent;
		hdr.hInstance = 0;
		hdr.nPages = (UINT)Pages.size();
		hdr.nStartPage = 0;
		hdr.ppsp = Pages.data();
		hdr.pszCaption = L"Save as...";
		if (s.func == 1)
			hdr.pszCaption = L"Open...";

		// Browser Emulation
		std::vector<wchar_t> fn(1000);
		GetModuleFileName(0, fn.data(), 1000);
		PathStripPath(fn.data());
		RGF::RKEY k(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");


		if (s.NoBrowserKey)
		{
			PropertySheet(&hdr);
		}
		else
		{
			k[fn.data()] = 11001UL;
			PropertySheet(&hdr);
			k[fn.data()].Delete();
		}
		return s.rs;
	}

	HRESULT Save(RGBF& s)
	{
		s.func = 0;
		return FunctionX(s);
	}

	HRESULT Open(RGBF& s)
	{
		s.func = 1;
		return FunctionX(s);
	}


}