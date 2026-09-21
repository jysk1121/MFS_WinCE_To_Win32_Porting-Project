#include "stdafx.h"

#include <fstream>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <stdio.h>

#include "lz4.h"
#include ".\Common\Cmnlib.h"

#ifdef UNDER_CE

using namespace std;

int WINAPI Lz4Compress(CString filePath)
{
	//read compressed binary file (assume we pass/encode nInputSize but don't know nCompressedSize)
	ifstream infSourceFile(filePath, ios::binary);

	//Get compressed file size for buffer
	infSourceFile.seekg (0,infSourceFile.end);
	int nInputSize = infSourceFile.tellg();
	wprintf(_T("inputsize: %d\n"), nInputSize);
	infSourceFile.clear();
	infSourceFile.seekg(0,ios::beg);

	//Read file into buffer
	char* pchInput = new char[nInputSize];
	infSourceFile.read(pchInput,nInputSize);
	infSourceFile.close();
	wprintf(_T("Read completed\n"));

	// compress szSource into pchCompressed
	char* pchCompressed = new char[nInputSize];
	int nCompressedSize = LZ4_compress_default((const char *)(pchInput), pchCompressed, nInputSize, nInputSize);
	wprintf(_T("compressed: %d\n"), nCompressedSize);

	// write pachCompressed to binary lz4.dat
	filePath += _T(".lz4");
	ofstream outBinaryFile(filePath, ofstream::binary);
	outBinaryFile.write(pchCompressed, nCompressedSize);
	outBinaryFile.close();
	delete[] pchCompressed;
	pchCompressed = 0;

	return 0;
}

int WINAPI Lz4Decompress(CString archivePath, CString dstPath)
{
	//read compressed binary file (assume we pass/encode nInputSize but don't know nCompressedSize)
	ifstream infCompressedBinaryFile(archivePath, ifstream::binary );

	//Get compressed file size for buffer
	infCompressedBinaryFile.seekg (0,infCompressedBinaryFile.end);
	int nCompressedInputSize = infCompressedBinaryFile.tellg();
	infCompressedBinaryFile.clear();
	infCompressedBinaryFile.seekg(0,ios::beg);

	//Read file into buffer
	char* pchCompressedInput = new char[nCompressedInputSize];
	infCompressedBinaryFile.read(pchCompressedInput, nCompressedInputSize);
	infCompressedBinaryFile.close();

	// Decompress buffer
	int nDecompressedSize = 0;
	int nInputSize = (1024 * 1024);
	char* pchDeCompressed = new char[nInputSize]; //(nCompressedInputSize *2) +8
	LZ4_decompress_safe(pchCompressedInput, pchDeCompressed, nDecompressedSize, nInputSize);
	delete[] pchCompressedInput;
	pchCompressedInput = 0;

	// write decompressed pachUnCompressed to
	ofstream outFile(dstPath, ios::binary | ios::out);
	outFile.write(pchDeCompressed, nInputSize);
	outFile.close();

	delete[] pchDeCompressed;
	pchDeCompressed = 0;

	return 0;
}

#endif // UNDER_CE