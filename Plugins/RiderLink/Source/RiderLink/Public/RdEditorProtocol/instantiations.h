#ifndef INSTANTIATIONS_H
#define INSTANTIATIONS_H

#include "serialization/Polymorphic.h"

#include "UE4TypesMarshallers.h"
#include "Runtime/Core/Public/Containers/Array.h"
#include "Runtime/Core/Public/Containers/ContainerAllocationPolicies.h"

extern template class rd::Wrapper<std::wstring>;

namespace ELogVerbosity {
    enum Type : uint8;
};

namespace rd {
    template <>
    class Polymorphic<ELogVerbosity::Type> {
        public:
        static ELogVerbosity::Type read(SerializationCtx& ctx, Buffer& buffer);
        static void write(SerializationCtx& ctx, Buffer& buffer, ELogVerbosity::Type const& value);
    };
    
    extern template class Polymorphic<ELogVerbosity::Type>;
    
};

#endif // INSTANTIATIONS_H
