#ifndef UDPFWDPROTO_H
#define UDPFWDPROTO_H

#include <QObject>
#include <QDataStream>

#include <cryptopp/eccrypto.h>
#include <cryptopp/sha3.h>
#include <cryptopp/gcm.h>
#include <cryptopp/aes.h>
#include <cryptopp/oids.h>

namespace UdpFwdProto {

constexpr quint16 DefaultPort = 13119;

using PrivateKey = CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP>;
using PublicKey = CryptoPP::DL_PublicKey_EC<CryptoPP::ECP>;

using SignatureScheme = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA3_256>;
using EncryptionScheme = CryptoPP::ECIES<CryptoPP::ECP, CryptoPP::SHA3_256>;
using E2EScheme = CryptoPP::GCM<CryptoPP::AES>;

Q_UDP_FWD_PROTOCOL_EXPORT void registerTypes();

Q_UDP_FWD_PROTOCOL_EXPORT CryptoPP::OID defaultCurve();

Q_UDP_FWD_PROTOCOL_EXPORT QDataStream &operator<<(QDataStream &stream, const CryptoPP::CryptoMaterial &key); // TODO move to CryptoQQ
Q_UDP_FWD_PROTOCOL_EXPORT QDataStream &operator>>(QDataStream &stream, CryptoPP::CryptoMaterial &key);

template <typename TValue>
QDataStream &operator<<(QDataStream &stream, const std::optional<TValue> &opt) {
	if (opt)
		stream << true << *opt;
	else
		stream << false;
	return stream;
}

template <typename TValue>
QDataStream &operator>>(QDataStream &stream, std::optional<TValue> &opt) {
	stream.startTransaction();
	bool valid;
	stream >> valid;
	if (valid) {
		opt = TValue{}; // initialize opt to be not null
		stream >> *opt;
	} else
		opt = std::nullopt;
	stream.commitTransaction();
	return stream;
}

Q_UDP_FWD_PROTOCOL_EXPORT QByteArray fingerPrint(const PublicKey &key);
Q_UDP_FWD_PROTOCOL_EXPORT QByteArray fingerPrint(const PrivateKey &key);

Q_UDP_FWD_PROTOCOL_EXPORT std::optional<PrivateKey> loadPrivateKey(QIODevice *device, CryptoPP::RandomNumberGenerator &rng);
Q_UDP_FWD_PROTOCOL_EXPORT std::optional<PublicKey> loadPublicKey(QIODevice *device, CryptoPP::RandomNumberGenerator &rng);
Q_UDP_FWD_PROTOCOL_EXPORT void savePrivateKey(QIODevice *device, const PrivateKey &key);
Q_UDP_FWD_PROTOCOL_EXPORT void savePublicKey(QIODevice *device, const PublicKey &key);
Q_UDP_FWD_PROTOCOL_EXPORT PrivateKey generateKey(CryptoPP::RandomNumberGenerator &rng, const CryptoPP::OID &curve = defaultCurve());

}

namespace CryptoPP {

Q_UDP_FWD_PROTOCOL_EXPORT bool operator!=(const UdpFwdProto::PublicKey &lhs, const UdpFwdProto::PublicKey &rhs);

}

Q_DECLARE_METATYPE(UdpFwdProto::PublicKey)
Q_DECLARE_METATYPE(UdpFwdProto::PrivateKey)
Q_DECLARE_METATYPE(std::optional<UdpFwdProto::PublicKey>)
Q_DECLARE_METATYPE(std::optional<UdpFwdProto::PrivateKey>)

#endif // UDPFWDPROTO_H
