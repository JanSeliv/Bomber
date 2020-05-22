#include "instantiations.h"

template class rd::Wrapper<std::wstring>;

namespace rd {
    ELogVerbosity::Type Polymorphic<ELogVerbosity::Type, void>::read(SerializationCtx& ctx, Buffer& buffer) {
        int32_t x = buffer.read_integral<int32_t>();
        switch (x) {
            case 10: 
               return ELogVerbosity::Type::VerbosityMask;
            case 11: 
               return ELogVerbosity::Type::SetColor;
            case 12: 
               return ELogVerbosity::Type::BreakOnLog;
            default:
               return static_cast<ELogVerbosity::Type>(x);
        }
    }
    
    void Polymorphic<ELogVerbosity::Type, void>::write(SerializationCtx& ctx, Buffer& buffer, ELogVerbosity::Type const& value) {
        switch (value) {
            case ELogVerbosity::Type::VerbosityMask: {
               buffer.write_integral<int32_t>(10);
               return;
            }
            case ELogVerbosity::Type::SetColor: {
               buffer.write_integral<int32_t>(11);
               return;
            }
            case ELogVerbosity::Type::BreakOnLog: {
               buffer.write_integral<int32_t>(12);
               return;
            }
            default:
            buffer.write_integral<int32_t>(static_cast<int32_t>(value));
        }
    }
    template class Polymorphic<ELogVerbosity::Type>;
};
