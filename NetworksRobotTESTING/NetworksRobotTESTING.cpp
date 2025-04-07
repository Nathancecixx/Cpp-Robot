#include "pch.h"
#include "CppUnitTest.h"
#include "PktDef.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            template<>
            std::wstring ToString<CmdType>(const CmdType& q) {
                switch (q) {
                case DRIVE:    return L"DRIVE";
                case SLEEP:    return L"SLEEP";
                case RESPONSE: return L"RESPONSE";
                default:       return L"Unknown CmdType";
                }
            }
        }
    }
}

namespace TestPktDef
{
    TEST_CLASS(UnitTestPktDef)
    {
    public:

        //test 1 - default constructor should initialize to safe state
        TEST_METHOD(DefaultConstructorTest)
        {
            PktDef pkt;
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(0, pkt.GetLength());
            Assert::IsFalse(pkt.GetAck());
            Assert::IsNull(pkt.GetBodyData());
        }

        //test 2 - testing the setters and getters
        TEST_METHOD(SettersAndGettersTest)
        {
            PktDef pkt;
            pkt.SetPktCount(123);
            Assert::AreEqual(123, pkt.GetPktCount());

            const char* testData = "Hello";
            pkt.SetBodyData(const_cast<char*>(testData), 5);
            // the expected length = HEADERSIZE + 5 + CRCSIZE
            Assert::AreEqual(HEADERSIZE + 5 + CRCSIZE, pkt.GetLength());
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), testData, 5));

            pkt.SetCmd(DRIVE);
            Assert::AreEqual(DRIVE, pkt.GetCmd());
        }

        //test 3 - varify CRC passes for a valid packet
        TEST_METHOD(CRCTest)
        {
            PktDef pkt;
            pkt.SetPktCount(1);
            const char* testData = "ABC";
            pkt.SetBodyData(const_cast<char*>(testData), 3);
            pkt.SetCmd(DRIVE);

            //generate the packet so that RawBuffer is updated
            char* raw = pkt.GenPacket();
            
            //calculate the CRC
            pkt.CalcCRC();
            int totalSize = pkt.GetLength();
            Assert::IsTrue(pkt.CheckCRC(raw, totalSize));
        }

        

        //test 4 - zero length body 
        TEST_METHOD(ZeroLengthBodyTest)
        {
            PktDef pkt;
            pkt.SetCmd(SLEEP);
            pkt.SetPktCount(42);
            pkt.SetBodyData("", 0);

            //the expected length = HEADERSIZE + CRCSIZE
            Assert::AreEqual(HEADERSIZE + 0 + CRCSIZE, pkt.GetLength());
            Assert::IsNotNull(pkt.GetBodyData());
            Assert::AreEqual('\0', pkt.GetBodyData()[0]);
            Assert::AreEqual(SLEEP, pkt.GetCmd());
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        //test 5 - testing large body data for whatever reason
        TEST_METHOD(LargeBodyDataTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            pkt.SetPktCount(999);

            const int bodySize = 50;
            char largeData[bodySize + 1];
            for (int i = 0; i < bodySize; i++)
                largeData[i] = 'X';
            largeData[bodySize] = '\0';

            pkt.SetBodyData(largeData, bodySize);

            Assert::AreEqual(HEADERSIZE + bodySize + CRCSIZE, pkt.GetLength());
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), largeData, bodySize));
            Assert::AreEqual(DRIVE, pkt.GetCmd());
            Assert::AreEqual(999, pkt.GetPktCount());
        }

        //test 6 - testing changing commands in sequance
        TEST_METHOD(MultipleCommandsTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            Assert::AreEqual(DRIVE, pkt.GetCmd());

            pkt.SetCmd(SLEEP);
            Assert::AreEqual(SLEEP, pkt.GetCmd());

            pkt.SetCmd(RESPONSE);
            Assert::AreEqual(RESPONSE, pkt.GetCmd());
        }

        //test 7- testing that setting body data multiple times works
        TEST_METHOD(BodyDataOverwriteTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);

            const char* data1 = "FirstData";
            pkt.SetBodyData(const_cast<char*>(data1), 9);
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), data1, 9));

            const char* data2 = "SecondData";
            pkt.SetBodyData(const_cast<char*>(data2), 10);
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), data2, 10));
        }

        //test 8 - testing that PktCount wraps with no issues
        TEST_METHOD(OverBoundaryPktCountTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            pkt.SetPktCount(70000);
            // 70000 mod 65536 = 70000 - 65536 = 4464
            Assert::AreEqual(4464, pkt.GetPktCount());
        }

        //test 9 - negative body size handling test for whatever reason
        TEST_METHOD(NegativeBodySizeTest)
        {
            PktDef pkt;
            pkt.SetCmd(SLEEP);
            try {
                pkt.SetBodyData("", -5);
                Assert::IsTrue(pkt.GetLength() >= (HEADERSIZE + CRCSIZE));
            }
            catch (...) {
                Assert::Fail();
            }
        }
    };
}
