#include "stdafx.h"
#include "rgf1.hpp"
#include "rgf2.hpp"


using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Core;
using namespace Windows::UI::Text;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Media::Core;
using namespace Windows::UI::Xaml::Markup;




#include "xml\\xml3all.h"

int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
	{
	winrt::init_apartment(apartment_type::single_threaded);
	WindowsXamlManager windowsXamlManager = WindowsXamlManager::InitializeForCurrentThread();
	WSADATA wData;
	WSAStartup(MAKEWORD(2, 2), &wData);
	INITCOMMONCONTROLSEX icex = { 0 };
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
	icex.dwSize = sizeof(icex);
	InitCommonControlsEx(&icex);
	InitCommonControls();

	std::vector<char> d(1024 * 1024 * 3);
	RGF::RGBF s;
	s.d = d.data();
	s.sz = d.size();
	s.resultFile = L"r:\\1.dat";
	s.DefExt = L"dat";


	XML3::XML rr("rgf.xml");

	#include "secret.h"
	/*
		fill s.google.id, s.google.secret, s.onedrive.id and s.onedrive.secret with your APP ClientID/Secret codes
	s.google.id  = "...";
	s.google.secret = "...";
	s.onedrive.id = "...";
	s.onedrive.secret = "...";

	*/

	// load tokens if we have them
	s.google.tokens.resize(3);
	s.onedrive.tokens.resize(3);
	s.google.tokens[0] = rr.GetRootElement()["tokens"]["google"].vv("t1").GetValue();
	s.google.tokens[1] = rr.GetRootElement()["tokens"]["google"].vv("t2").GetValue();
	s.onedrive.tokens[0] = rr.GetRootElement()["tokens"]["onedrive"].vv("t1").GetValue();
	s.onedrive.tokens[1] = rr.GetRootElement()["tokens"]["onedrive"].vv("t2").GetValue();


	auto rv = RGF::Save(s);

	// Save tokens if we got them
	if (s.google.tokens.size() == 3)
	{
		rr.GetRootElement()["tokens"]["google"].vv("t1").SetValue(s.google.tokens[0].c_str());
		rr.GetRootElement()["tokens"]["google"].vv("t2").SetValue(s.google.tokens[1].c_str());
	}
	if (s.onedrive.tokens.size() == 3)
	{
		rr.GetRootElement()["tokens"]["onedrive"].vv("t1").SetValue(s.onedrive.tokens[0].c_str());
		rr.GetRootElement()["tokens"]["onedrive"].vv("t2").SetValue(s.onedrive.tokens[1].c_str());
	}
	rr.Save();

	RGF::RGBF s2 = s;

	// Test open file
	std::vector<char> r;
	s2.read = &r;
	s2.Filter.clear();
	s2.resultFile.clear();
	rv = RGF::Open(s2);

	return 0;
	}