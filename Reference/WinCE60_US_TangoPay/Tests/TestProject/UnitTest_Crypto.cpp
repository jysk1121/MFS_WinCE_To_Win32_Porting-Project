#include "stdafx.h"
#include <iostream>
#include <afx.h>
#include "..\..\App\H\Common\NHDbgApi.h"
#include "..\..\App\H\Common\CmnLib.h"
#include "..\..\App\H\Common\Crypto.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

namespace TestProject
{
	[TestClass]
	public ref class UnitTest_Crypto
	{
	private:
		TestContext^ testContextInstance;

	public: 
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

		#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
		#pragma endregion 

		[TestMethod]
		void TestMethod1()
		{
			//
			// TODO: Add test logic	here
			//
			NHDBG((L"All is %s.", _T("whell")));
		};

		[TestMethod]
		void TestMethod_GenerateUUID()
		{
			CString uuidValue = CryptoLib::GetNewUUID();
			NHDBG((L"A new UUID generated: '%s'\n", uuidValue));
			Assert::IsFalse(uuidValue.IsEmpty());

			CString uuidValueWithoutHyphens = CryptoLib::GetNewUUID(false);
			NHDBG((L"A new UUID without hyphens generated: '%s'\n", uuidValueWithoutHyphens));
			Assert::IsFalse(uuidValueWithoutHyphens.IsEmpty());
		};

		[TestMethod]
		void TestMethod_Base64Encode()
		{
			CString data = L"bffdcff17bed7037b2b015222a98a8a27ad83aae7ff9382f8e9d2ca09b53a212";
			CString expectedResult = L"v/3P8XvtcDeysBUiKpioonrYOq5/+Tgvjp0soJtTohI=";

			CString base64encoded = CryptoLib::EncodeToBase64(data);
			NHDBG((L"base64encoded : %s\n", base64encoded));

			Assert::IsTrue(base64encoded == expectedResult);
		};

		[TestMethod]
		void TestMethod_Base64UrlEncode()
		{
			CString data = L"bffdcff17bed7037b2b015222a98a8a27ad83aae7ff9382f8e9d2ca09b53a212";
			CString expectedResult = L"v_3P8XvtcDeysBUiKpioonrYOq5_-Tgvjp0soJtTohI";

			CString base64encoded = CryptoLib::EncodeToBase64(data, true);
			NHDBG((L"base64encoded : %s\n", base64encoded));

			Assert::IsTrue(base64encoded == expectedResult);
		};

		[TestMethod]
		void TestMethod_GenerateHMACSHA256()
		{
			CString strhmacKey = L"54dfeb2a27424db1b9879ad325fa0772";
			CString nonceId = L"d230b3dfd007484ca712acbc580f6503D0000003_1";
			CString exepctedChecksum = L"v_3P8XvtcDeysBUiKpioonrYOq5_-Tgvjp0soJtTohI";

			CString strCheckSum = CryptoLib::GenerateHMACSHA256(nonceId, strhmacKey);
			NHDBG((L"strCheckSum : %s\n", strCheckSum));

			CString strCheckSumBase64Encoded = CryptoLib::EncodeToBase64(strCheckSum, true);
			NHDBG((L"strCheckSumBase64Encoded (base64) : %s\n", strCheckSumBase64Encoded));

			Assert::IsTrue(strCheckSumBase64Encoded == exepctedChecksum);
		};

		[TestMethod]
		void TestMethod_RSAEncryptionWithPublicKey()
		{
			CString strhmacKey = L"54dfeb2a27424db1b9879ad325fa0772";
			CString publicKey = L"-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAx/EI/H0eVVgGZGD8vB/8\nok4keIweQgHgJdBVD6AJeQIxmNXNEagSzS+tefYI5Vc36BpRK6J16eW28twDIVxT\njYzM192I9zDJP9Ae+T+f8+FOKuMdYQBOfqquQDf3jmm096IIkZo2bMTKXnr7keSH\ndVi3WsRNittJN4Qm6vL+IzHbUYhfAKvX5KFe90qY7dOW0bn5JTR9X+baGwg9R1iO\nm5iE0gt+ckZG82Ir1zpmlFck27x+UWfVUQxpazkiwJZHnS1Fha8bxRoVyc2cl0+m\nOkMcFVaGj5ssuX1Lq04YUYhVXzo4hwZfxsf0nCyvmOgHaci44aTCEf0RjetSymI5\nZQIDAQAB\n-----END PUBLIC KEY-----";

			CString encryptedHMacKey = CryptoLib::RSAEncryptWithPublicKey(strhmacKey, publicKey);

			Assert::IsFalse(encryptedHMacKey.IsEmpty());
		};
	};
}
