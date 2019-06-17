#ifndef MESSAGE_P_H
#define MESSAGE_P_H

#include <udpfwdproto.h>

#include <cryptopp/filters.h>
#include <cryptopp/secblock.h>

namespace UdpFwdProto::__internal {

class SecByteBlockSink : public CryptoPP::Bufferless<CryptoPP::Sink>
{
	// Code from: https://www.cryptopp.com/wiki/SecByteBlockSink
public:
	SecByteBlockSink(CryptoPP::SecByteBlock& sbb);

	size_t Put2(const CryptoPP::byte *inString, size_t length, int /*messageEnd*/, bool /*blocking*/) override;

private:
	CryptoPP::SecByteBlock& _sbb;
};

class SignatureStream : public QDataStream
{
public:
	SignatureStream();

	QByteArray sign(CryptoPP::RandomNumberGenerator &rng, const PrivateKey &key) const;
	bool verify(QByteArray signature, const PublicKey &key) const;

private:
	QByteArray _sigData;
};

}

#endif // MESSAGE_P_H
