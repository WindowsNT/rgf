// rgf

namespace RGF
{

	struct ASKTEXT
	{
		const wchar_t* ti;
		const wchar_t* as;
		wchar_t* re;
		int P;
		std::wstring* re2;
		int mx = 1000;
	};

	inline INT_PTR CALLBACK ASKTEXT_DP(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
	{
		static ASKTEXT* as = 0;
		switch (mm)
		{
		case WM_INITDIALOG:
		{
			as = (ASKTEXT*)ll;
			SetWindowText(hh, as->ti);
			if (as->P != 2)
			{
				SetWindowText(GetDlgItem(hh, 101), as->as);
				if (as->re)
					SetWindowText(GetDlgItem(hh, 102), as->re);
				if (as->re2)
					SetWindowText(GetDlgItem(hh, 102), as->re2->c_str());
			}
			else
				SetWindowText(GetDlgItem(hh, 701), as->as);
			if (as->P == 1)
			{
				auto w = GetWindowLongPtr(GetDlgItem(hh, 102), GWL_STYLE);
				w |= ES_PASSWORD;
				SetWindowLongPtr(GetDlgItem(hh, 102), GWL_STYLE, w);
			}
			return true;
		}
		case WM_COMMAND:
		{
			if (LOWORD(ww) == IDOK)
			{
				wchar_t re1[1000] = { 0 };
				wchar_t re2[1000] = { 0 };
				//					MessageBox(0, L"foo", 0, 0);
				if (as->P == 2)
				{
					GetWindowText(GetDlgItem(hh, 101), re1, 1000);
					GetWindowText(GetDlgItem(hh, 102), re2, 1000);
					if (wcscmp(re1, re2) != 0 || wcslen(re1) == 0)
					{
						SetWindowText(GetDlgItem(hh, 101), L"");
						SetWindowText(GetDlgItem(hh, 102), L"");
						SetFocus(GetDlgItem(hh, 101));
						return 0;
					}
					wcscpy_s(as->re, 1000, re1);
					EndDialog(hh, IDOK);
					return 0;
				}

				if (as->re2)
				{
					int lex = GetWindowTextLength(GetDlgItem(hh, 102));
					std::vector<wchar_t> re(lex + 100);
					GetWindowText(GetDlgItem(hh, 102), re.data(), lex + 100);
					*as->re2 = re.data();
					EndDialog(hh, IDOK);
				}
				else
				{
					GetWindowText(GetDlgItem(hh, 102), as->re, as->mx);
					EndDialog(hh, IDOK);
				}
				return 0;
			}
			if (LOWORD(ww) == IDCANCEL)
			{
				EndDialog(hh, IDCANCEL);
				return 0;
			}
		}
		}
		return 0;
	}

	inline bool AskText(HWND hh, const TCHAR* ti, const TCHAR* as, TCHAR* re, std::wstring* re2 = 0, int mxt = 1000)
	{
		const char* res = "\xC4\x0A\xCA\x90\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x6D\x01\x3E\x00\x00\x00\x00\x00\x00\x00\x0A\x00\x54\x00\x61\x00\x68\x00\x6F\x00\x6D\x00\x61\x00\x00\x00\x01\x00\x00\x50\x00\x00\x00\x00\x0A\x00\x09\x00\x1C\x01\x1A\x00\x65\x00\xFF\xFF\x82\x00\x00\x00\x00\x00\x00\x00\x80\x00\x81\x50\x00\x00\x00\x00\x0A\x00\x29\x00\x1D\x01\x0F\x00\x66\x00\xFF\xFF\x81\x00\x00\x00\x00\x00\x00\x00\x00\x03\x01\x50\x00\x00\x00\x00\x2F\x01\x16\x00\x32\x00\x0E\x00\x01\x00\xFF\xFF\x80\x00\x4F\x00\x4B\x00\x00\x00\x00\x00\x00\x00\x00\x03\x01\x50\x00\x00\x00\x00\x2F\x01\x29\x00\x32\x00\x0E\x00\x02\x00\xFF\xFF\x80\x00\x43\x00\x61\x00\x6E\x00\x63\x00\x65\x00\x6C\x00\x00\x00\x00\x00";
		ASKTEXT a = { ti,as,re,0,re2,mxt };
		return (DialogBoxIndirectParam(GetModuleHandle(0), (LPCDLGTEMPLATEW)res, hh, ASKTEXT_DP, (LPARAM)& a) == IDOK);
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
				ShowWindow(GetDlgItem(GetParent(hh),IDOK),SW_HIDE);
				SetWindowLongPtr(hh, GWLP_USERDATA, ll);
				p = (PROPSHEETPAGE*)ll;
				r = (RGF::RGBF*)p->lParam;
				r->hH = hh;

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


	void GoogleThreadLoad2(RGBF* r, GOD::ystring NewName, std::string NewID,HWND hh)
	{
		// We have data
		std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
		std::string j = r->goo->dir(r->google.root.c_str(), true, r->func == 0 ? true : false);
		RGF::GOD::EnumNames(*r->goo, j, &AllItems, 1, !r->func);
		SendMessage(hh, WM_USER + 501, (WPARAM)& NewName, (LPARAM)& AllItems);
	};


	void GenericDialogState(HWND hh,int s)
	{
		if (s == 0) // waiting
		{
			ShowWindow(GetDlgItem(hh, 701), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 801), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 901), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 101), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 202), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 211), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 212), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 213), SW_HIDE);
		}
		else
		{
			ShowWindow(GetDlgItem(hh, 701), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 801), SW_HIDE);
			ShowWindow(GetDlgItem(hh, 901), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 101), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 202), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 211), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 212), SW_SHOW);
			ShowWindow(GetDlgItem(hh, 213), SW_SHOW);

		}
	}

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
			r->hH = hh;
			if (r->func == 1)
				DestroyWindow(GetDlgItem(hh, 202));
			SendDlgItemMessage(hh, 801, PBM_SETMARQUEE, true, 0);
			DWORD LEST = LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
			ListView_SetExtendedListViewStyle(hL, LEST);
			ListInsertColumn(hL, 0, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 30, L"Name", 0, 0, 0, 0);
			ListInsertColumn(hL, 1, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 0, L"ID", 0, 0, 0, 0);
			ListInsertColumn(hL, 2, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 0, L"MIME", 0, 0, 0, 0);

			GenericDialogState(hh,0);

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
				std::thread t(GoogleThreadLoad2, r, "root", "root",hh);
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
			if (n->code == PSN_QUERYCANCEL)
			{
				if (r->InProgress)
				{
					r->ShouldCancelProp = true;
					SetWindowLongPtr(hh, DWLP_MSGRESULT, true);
				}
				return 1;
			}

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
					GenericDialogState(hh, 0);
					ListView_DeleteAllItems(hL);

					GOD::ystring NewName = GOD::ystring(t1.data()).a_str();
					std::string NewID = GOD::ystring(t2.data()).a_str();
					std::thread t(GoogleThreadLoad2, r, NewName, NewID,hh);
					t.detach();

				}
				else
				{
					// Download
					GenericDialogState(hh, 0);
					SetDlgItemText(hh, 701, L"Opening...");
					GOD::ystring fid = t2.data();
					r->resultFile = fid;
					DWORD st = (DWORD)GetWindowLongPtr(GetDlgItem(hh, 801), GWL_STYLE);
					st &= ~PBS_MARQUEE;
					SetWindowLong(GetDlgItem(hh, 801), GWL_STYLE, st);

					if (r->read)
					{
						auto foo = [](GOD::ystring fid, RGF::RGBF* r)
						{
							r->InProgress = true;
							auto hr = r->goo->Download(fid.a_str(), 0, r->read, 0, (unsigned long long) - 1, [](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
								{
									RGF::RGBF* s = (RGF::RGBF*)lp;
									f *= 100;
									f = (int)(f / t);
									SendDlgItemMessage(s->hH, 801, PBM_SETPOS, (WPARAM)f, 0);
									if (s->ShouldCancelProp)
										return E_FAIL;
									return S_OK;
								}, r);
							r->rs = hr;
							r->InProgress = false;
							SendMessage(r->hH, WM_CLOSE, 0, 0);
						};

						std::thread tx(foo, fid, r);
						tx.detach();
						return 0;
					}
					SendMessage(r->hH, WM_CLOSE, 0, 0);
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
				GenericDialogState(hh, 0);
				r->google.root = r->goo->GetRootFolderID();
				std::thread t(GoogleThreadLoad2, r, "root", "root",hh);
				t.detach();
			}

			if (LW == 212)
			{
				// New folder
				std::vector<wchar_t> nf(1000);
				if (!AskText(hh, L"New folder", L"New folder name:", nf.data()))
					return 0;
				RGF::GOD::ystring nff = nf.data();
				r->goo->CreateFolder(nff.a_str(), r->google.root.c_str());
				GenericDialogState(hh, 0);
				std::thread t(GoogleThreadLoad2, r, nff, r->google.root,hh);
				t.detach();
			}

			if (LW == 213)
			{
				// Logout
				r->google.tokens[0] = "";
				r->google.tokens[1] = "";
				r->goo->Unauth();
				r->goo = nullptr;
				SendMessage(GetParent(hh), PSM_SETCURSEL, 0, 0);
			}


			// Save
			if (LW == 202)
			{
				GenericDialogState(hh, 0);
				std::vector<wchar_t> t(1000);
				GetDlgItemText(hh, 101, t.data(), 1000);
				std::wstring fi = t.data();
				if (fi.empty())
					return 0;

				if (wcschr(fi.c_str(), L'.') == 0)
				{
					if (r->DefExt.length())
					{
						fi += L".";
						fi += r->DefExt;
					}
				}

				// We upload to google
				SendMessage(GetDlgItem(hh, 801), PBM_SETMARQUEE, 0, 0);
				SetDlgItemText(hh,701,L"Saving...");
				DWORD st = (DWORD)GetWindowLongPtr(GetDlgItem(hh, 801), GWL_STYLE);
				st &= ~PBS_MARQUEE;
				SetWindowLong(GetDlgItem(hh, 801), GWL_STYLE, st);
				r->InProgress = true;
				auto up = [](GOD::ystring fi, RGF::RGBF* s)
				{
					std::string ret;
					auto hr = s->goo->UploadOnce(0, s->d, s->sz, s->google.root.c_str(), fi.a_str(), ret,
						[](unsigned long long f, unsigned long long t, void* lp) -> HRESULT
						{
							RGBF* s = (RGBF*)lp;
							f *= 100;
							f = (int)(f / t);
							SendDlgItemMessage(s->hH,801, PBM_SETPOS, (WPARAM)f, 0);
							if (s->ShouldCancelProp)
								return E_FAIL;
							return S_OK;
						}
					, s);
					s->InProgress = false;
					s->rs = hr;
					if (SUCCEEDED(hr))
						s->resultFile = ret;
					SendMessage(s->hH, WM_CLOSE, 0, 0);
				};

				GOD::ystring y = fi.c_str();
				std::thread tf(up, y, r);
				tf.detach();
			}



			return 0;
		}


		case WM_USER + 501:
		{
			// Fill Google Items
			// ll = &vector<tuple<string,string>>
			// ww = &string current root
			ListView_DeleteAllItems(hL);
			GenericDialogState(hh, 1);

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
					if (r->func == 1)
					{
						_stprintf_s(tt.data(), 1000, _T("%s"), y1.c_str());
						LV_ITEM lv = { 0 };
						lv.mask = LVIF_TEXT | LVIF_PARAM;
						lv.iItem = ListView_GetItemCount(hL);
						lv.pszText = tt.data();
						lv.lParam = (LPARAM)lv.iItem;
						int L = ListView_InsertItem(hL, &lv);

						ListView_SetItemText(hL, L, 1, (LPWSTR)y2.c_str());
						ListView_SetItemText(hL, L, 2, (LPWSTR)mi.c_str());
					}
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


	auto PrjWiz4DP = [](HWND hh, UINT mm, WPARAM ww, LPARAM ll) ->INT_PTR
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



	HRESULT FunctionX2(RGBF& s)
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
		// OneDrive
		if (true)
		{
			PROPSHEETPAGE p1 = { 0 };
			p1.dwSize = sizeof(p1);
			p1.pszHeaderTitle = L"OneDrive";
			p1.lParam = 0;
			p1.hInstance = GetModuleHandle(0);
			p1.pszTemplate = L"DIALOG_RGF_ONE";
			p1.pfnDlgProc = PrjWiz3DP;
			p1.lParam = (LPARAM)& s;
			Pages.push_back(p1);
		}
		// DropBox
		if (true)
		{
			PROPSHEETPAGE p1 = { 0 };
			p1.dwSize = sizeof(p1);
			p1.pszHeaderTitle = L"DropBox";
			p1.lParam = 0;
			p1.hInstance = GetModuleHandle(0);
			p1.pszTemplate = L"DIALOG_RGF_DROPBOX";
			p1.pfnDlgProc = PrjWiz4DP;
			p1.lParam = (LPARAM)& s;
			Pages.push_back(p1);
		}

		PROPSHEETHEADER hdr = { 0 };
		hdr.dwSize = sizeof(hdr);
		hdr.dwFlags = PSH_PROPSHEETPAGE  | PSH_NOAPPLYNOW;
		hdr.hwndParent = s.hParent;
		hdr.hInstance = 0;
		hdr.nPages = (UINT)Pages.size();
		hdr.nStartPage = 0;
		hdr.pfnCallback = 0;
//		hdr.pfnCallback
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

	HRESULT Save2(RGBF& s)
	{
		s.func = 0;
		return FunctionX2(s);
	}

	HRESULT Open2(RGBF& s)
	{
		s.func = 1;
		return FunctionX2(s);
	}


}