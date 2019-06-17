#ifndef UDPFWDPROTO_P_H
#define UDPFWDPROTO_P_H

#include "udpfwdproto.h"

#include <tuple>
#include <variant>

#include <QMetaProperty>
#include <QVariant>
#include <QScopedPointer>

namespace UdpFwdProto::__internal {

template <typename TVariant>
TVariant deserIf(quint8, QDataStream &stream) {
	stream.rollbackTransaction();
	return {};
}

template <typename TVariant, typename TMessage, typename... TArgs>
TVariant deserIf(quint8 messageCode, QDataStream &stream) {
	if (messageCode == TMessage::MessageCode) {
		stream.startTransaction();
		const auto mo = &TMessage::staticMetaObject;
		TMessage message;
		for (auto i = 0; i < mo->propertyCount(); ++i) {
			const auto prop = mo->property(i);
			if (prop.isStored()) {
				QVariant val{prop.userType(), nullptr};
				if (!QMetaType::load(stream, prop.userType(), val.data())) {
					stream.abortTransaction();
					return {};
				}
				if (!prop.writeOnGadget(&message, val)) {
					stream.abortTransaction();
					return {};
				}
			}
		}
		return message;
	} else
		return deserIf<TVariant, TArgs...>(messageCode, stream);
}



template <typename TObject, typename TMessageRef>
using CbMemPtr = void (TObject::*)(TMessageRef);

template <typename TObject, typename TMessage>
class MemBinderBase {
public:
	virtual inline ~MemBinderBase() = default;
	virtual void operator()(TObject *object, TMessage &&message) const = 0;
};

template <typename TObject, typename TMessageRef>
class MemBinder : public MemBinderBase<TObject, std::decay_t<TMessageRef>> {
public:
	MemBinder(CbMemPtr<TObject, TMessageRef> member) :
		_member{member}
	{}

	void operator()(TObject *object, std::decay_t<TMessageRef> &&message) const override {
		(object->*_member)(std::move(message));
	}

private:
	CbMemPtr<TObject, TMessageRef> _member;
};

template <typename TObject, typename... TMessages>
class CallableTuple : public std::tuple<QScopedPointer<MemBinderBase<TObject, TMessages>>...>
{
public:
	template <typename TMessage>
	using Element = QScopedPointer<MemBinderBase<TObject, TMessage>>;

	template <typename... TMessagesRef>
	CallableTuple(TObject *object, CbMemPtr<TObject, TMessagesRef>... callbacks) :
		std::tuple<Element<TMessages>...>(
			Element<std::decay_t<TMessagesRef>>{
				new MemBinder<TObject, TMessagesRef>(callbacks)
			}...
		),
		_object{object}
	{}

	template <typename TParam>
	void operator()(TParam&& param) {
		std::get<Element<TParam>>(*this)(_object, std::forward<TParam>(param));
	}

private:
	TObject *_object;
};

}

#endif // UDPFWDPROTO_P_H
