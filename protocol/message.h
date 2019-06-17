#ifndef MESSAGE_H
#define MESSAGE_H

#include "udpfwdproto.h"
#include "udpfwdproto_p.h"

#include <type_traits>
#include <limits>

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT Message
{
	Q_GADGET

public:
	template <typename TMessage>
	static QByteArray serialize(const TMessage &message);

	template <typename... TMessages>
	static std::variant<TMessages...> deserialize(const QByteArray &data);

	// member wrappers
	template <typename TObject, typename... TMessagesRef>
	void deserialize(const QByteArray &data,
					 TObject *object,
					 __internal::CbMemPtr<TObject, TMessagesRef>... callbacks);
};

// ------------- GENERIC IMPLEMENTATION -------------

template <typename TMessage>
QByteArray Message::serialize(const TMessage &message)
{
	static_assert (std::is_base_of_v<Message, TMessage>, "TMessage must extend UdpFwdProto::Message");
	const auto mo = &TMessage::staticMetaObject;
	QByteArray data;
	QDataStream stream{&data, QIODevice::WriteOnly};
	stream << TMessage::MessageCode;
	for (auto i = 0; i < mo->propertyCount(); ++i) {
		const auto prop = mo->property(i);
		if (prop.isStored()) {
			const auto val = prop.readOnGadget(&message);
			if (!QMetaType::save(stream, prop.userType(), val.data()))
				return {};
		}
	}
	return data;
}

template <typename... TMessages>
std::variant<TMessages...> Message::deserialize(const QByteArray &data)
{
	QDataStream stream{data};
	stream.startTransaction();
	quint8 msgCode;
	stream >> msgCode;
	auto result = __internal::deserIf<std::variant<TMessages...>, TMessages...>(msgCode, stream);
	if (stream.commitTransaction())
		return result;
	else
		throw std::bad_variant_access{};
}

template <typename TObject, typename... TMessagesRef>
void Message::deserialize(const QByteArray &data, TObject *object, __internal::CbMemPtr<TObject, TMessagesRef>... callbacks) {
	std::visit(__internal::CallableTuple<TObject, std::decay_t<TMessagesRef>...> {
		object, callbacks...
	}, deserialize<std::decay_t<TMessagesRef>...>(data));
}

}

Q_DECLARE_METATYPE(UdpFwdProto::Message)

#endif // MESSAGE_H
