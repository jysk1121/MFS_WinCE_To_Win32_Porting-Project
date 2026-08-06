#pragma once

#if defined(UNICODE) || defined(_UNICODE)
	#define tstring		wstring
	#define tout		wcout
	#define tin			wcin
	#define tostream	wostream
	#define tofstream	wofstream
	#define tifstream	wifstream
	#define tfstream	wfstream
#else
	#define tstring		string
	#define tout		cout
	#define tin			cin
	#define tostream	ostream
	#define tofstream	ofstream
	#define tifstream	ifstream
	#define tfstream	fstream
#endif
