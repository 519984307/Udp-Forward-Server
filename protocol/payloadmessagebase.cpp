#include "payloadmessagebase.h"
#include "message_p.h"
#include <qiodevicesource.h>
#include <qiodevicesink.h>
using namespace CryptoPP;
using namespace CryptoQQ;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

void PayloadMessageBase::createEncrypted(RandomNumberGenerator &rng, const PublicKey &key, const QByteArray &data, ReplyInfo replyInfo)
{
	EncryptionScheme::Encryptor keyEnc {key};
	E2EScheme::Encryption dataEnc;

	// encrypt the message
	SecByteBlock aesKey{dataEnc.GetValidKeyLength(keyEnc.FixedMaxPlaintextLength())};
	iv.resize(static_cast<int>(dataEnc.IVSize()));
	rng.GenerateBlock(aesKey, aesKey.size());
	rng.GenerateBlock(iv, iv.byteSize());
	dataEnc.SetKeyWithIV(aesKey, aesKey.size(),
						 iv, iv.byteSize());
	QByteArraySource {
		data, true, new AuthenticatedEncryptionFilter {
			dataEnc, new QByteArraySink {
				encryptedPayload
			}
		}
	};

	// encrypt the key
	StringSource {
		aesKey, aesKey.size(), true, new PK_EncryptorFilter {
			rng, keyEnc, new QByteArraySink {
				encryptedKey
			}
		}
	};

	this->replyInfo = std::move(replyInfo);
}

QByteArray PayloadMessageBase::decrypt(RandomNumberGenerator &rng, const UdpFwdProto::PrivateKey &key) const
{
	EncryptionScheme::Decryptor keyDec {key};
	E2EScheme::Decryption dataDec;

	// restore the key
	SecByteBlock aesKey;
	QByteArraySource {
		encryptedKey, true, new PK_DecryptorFilter {
			rng, keyDec, new SecByteBlockSink {
				aesKey
			}
		}
	};

	// restore the data
	QByteArray data;
	dataDec.SetKeyWithIV(aesKey, aesKey.size(),
						 iv, iv.byteSize());
	QByteArraySource {
		encryptedPayload, true, new AuthenticatedDecryptionFilter {
			dataDec, new QByteArraySink {
				data
			}, AuthenticatedDecryptionFilter::THROW_EXCEPTION
		}
	};

	return data;
}



ReplyInfo::ReplyInfo(PublicKey key, quint16 limit) :
	limit{limit},
	key{std::move(key)}
{}

ReplyInfo::operator bool() const
{
	return limit != 0;
}

bool ReplyInfo::operator!() const
{
	return limit == 0;
}

QDataStream &operator<<(QDataStream &stream, const ReplyInfo &info)
{
	stream << info.limit;
	if (info.limit != 0)
		stream << info.key;
	return stream;
}

QDataStream &operator>>(QDataStream &stream, ReplyInfo &info)
{
	stream >> info.limit;
	if (info.limit != 0)
		stream >> info.key;
	return stream;
}
