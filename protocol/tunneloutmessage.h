#ifndef TUNNELOUTMESSAGE_H
#define TUNNELOUTMESSAGE_H

#include "payloadmessagebase.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT TunnelOutMessage : public PayloadMessageBase
{
	Q_GADGET

public:
	static constexpr quint8 MessageCode = 0x04;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::TunnelOutMessage)

#endif // TUNNELOUTMESSAGE_H
