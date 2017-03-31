#include <QtTest>
#include <iostream>
#include "../arsenic/crypto/argon2/argon2.h"
#include "../arsenic/crypto/botan/botan_all.h"
#include "../arsenic/crypto/Crypto.h"
#include "../arsenic/crypto/hash_tool.h"

using namespace std;


class TestArsenic: public QObject
{
    Q_OBJECT
private slots:
    void testSimpleTests();
    void testArgon2();
    void testHash();
    void testEncrypt();
    void testDecrypt();
};

void TestArsenic::testSimpleTests()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

void TestArsenic::testHash()
{
    Hash_Tool hash;
    QStringList fileList;

    const QString fileName1 = QStringLiteral("test1.txt");

    QFile file1{fileName1};
    if (!file1.exists())
    {
        QString message;
        auto msg = QStringLiteral("Test file %1 is missing. ");

        if (!file1.exists())
        {
            message += msg.arg(fileName1);
        }

        QSKIP(message.toStdString().c_str());

    }
    fileList.append(fileName1);
    hash.setParam(fileList,"SHA-256");


    hash.start();
    hash.wait();

}

void TestArsenic::testArgon2()
{
    // for this references parameters and outputs, see :
    // https://github.com/P-H-C/phc-winner-argon2/blob/master/README.md

    uint32_t time_cost     = 2;
    uint32_t memory_cost   = 1<<16; // 65536 KiB
    uint32_t parallelism   = 4;

    string password  = "password";
    string salt      = "somesalt";

    string referenceResultEncoded = "$argon2i$v=19$m=65536,t=2,p=4$c29tZXNhbHQ$RdescudvJCsgt3ub+b+dWRWJTmaaJObG";
    string referenceResultRaw     = "45D7AC72E76F242B20B77B9BF9BF9D5915894E669A24E6C6";

    // ///////////////////////////////


    vector<uint8_t> outputRaw(24);
    vector<char> outputEncoded(256);

    argon2_hash(time_cost, memory_cost, parallelism,
                password.data(), password.size(),
                salt.data(), salt.size(),
                outputRaw.data(), outputRaw.size(),
                outputEncoded.data(), outputEncoded.size(),
                Argon2_i, ARGON2_VERSION_NUMBER);

    QVERIFY(outputEncoded.data() == referenceResultEncoded);
    QVERIFY(Botan::hex_encode(outputRaw) == referenceResultRaw);

}

void TestArsenic::testEncrypt()
{
    Crypto_Thread crypto;
    QStringList fileList;

    const QString fileName1 = QStringLiteral("test1.txt");

    QFile file1{fileName1};
    if (!file1.exists())
    {
        QString message;
        auto msg = QStringLiteral("Test file %1 is missing. ");

        if (!file1.exists())
        {
            message += msg.arg(fileName1);
        }

        QSKIP(message.toStdString().c_str());

    }
    QVERIFY(fileName1==fileName1);

    QString password = "my password";
    QString algo     = "Serpent/GCM";
    bool encrypt     = true;
    QString encoding = "Base64_Encoder";
    bool deleteOriginals     = false;
    QString appVersion= "0.9.9";

    fileList.append(fileName1);

    crypto.setParam(fileList, password, algo, encrypt, encoding, deleteOriginals, appVersion);
    crypto.start();
    crypto.wait();

    QFile file2{"test1.txt.ars"};
    QVERIFY(file2.exists());
}

void TestArsenic::testDecrypt()
{
    QString sha_256_original="9C759C9AA3CB6CE29A46C4EDA37FBFE29DA5F988FD5106A95B1377F657A3FD32";
    Crypto_Thread crypto;
    QStringList fileList;
    const QString fileName = QStringLiteral("test1.txt.ars");

    QFile file{fileName};
    if (!file.exists())
    {
        QString message;
        auto msg = QStringLiteral("encrypted Test file %1 is missing. ");

        if (!file.exists())
        {
            message += msg.arg(fileName);
        }

        QSKIP(message.toStdString().c_str());

    }
    QString password = "my password";
    QString algo     = "not used for decrypt";
    bool encrypt     = false;
    QString encoding = "not used for decrypt";
    bool deleteOriginals     = false;
    QString appVersion= "0.9.9";

    fileList.append(fileName);

    crypto.setParam(fileList, password, algo, encrypt, encoding, deleteOriginals, appVersion);
    crypto.start();
    crypto.wait();

    QFile file2{"test1 (2).txt"};
    QVERIFY(file2.exists());

}

QTEST_MAIN(TestArsenic)
#include "testsarsenic.moc"