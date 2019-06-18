#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include "message.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT ErrorMessage : public Message
{
	Q_GADGET

	Q_PROPERTY(Error error MEMBER error)

public:
	static constexpr quint8 MessageCode = 0x00;

	enum class Error {
		Unknown,
		InvalidPeer,
		InvalidSignature,
		InvalidKey
	};
	Q_ENUM(Error)

	Error error = Error::Unknown;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::ErrorMessage)

#endif // ERRORMESSAGE_H
