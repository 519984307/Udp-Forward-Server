#include "message.h"
#include "message_p.h"
#include <qiodevicesource.h>
#include <qiodevicesink.h>
using namespace CryptoPP;
using namespace CryptoQQ;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

SecByteBlockSink::SecByteBlockSink(SecByteBlock &sbb) :
	_sbb{sbb}
{}

size_t SecByteBlockSink::Put2(const byte *inString, size_t length, int, bool)
{
	if (!inString || !length)
		return length;

	auto wIndex = _sbb.size();
	_sbb.Grow(wIndex + length);
	std::memcpy(_sbb.data() + wIndex, inString, length);
	return 0;
}



SignatureStream::SignatureStream() :
	QDataStream{&_sigData, QIODevice::WriteOnly}
{}

QByteArray SignatureStream::sign(RandomNumberGenerator &rng, const PrivateKey &key) const {
	QByteArray signature;
	SignatureScheme::Signer signer{key};
	QByteArraySource {
		_sigData, true, new SignerFilter {
			rng, signer, new QByteArraySink {
				signature
			}
		}
	};
	return signature;
}

bool SignatureStream::verify(QByteArray signature, const PublicKey &key) const {
	signature.append(_sigData);
	SignatureScheme::Verifier verifier{key};
	auto ok = false;
	QByteArraySource {
		signature, true, new SignatureVerificationFilter {
			verifier, new ArraySink {
				reinterpret_cast<byte*>(&ok), sizeof (ok)
			}, SignatureVerificationFilter::PUT_RESULT | SignatureVerificationFilter::SIGNATURE_AT_BEGIN
		}
	};
	return ok;
}
