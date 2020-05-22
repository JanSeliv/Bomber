#pragma once

#include "UE4TypesMarshallers.h"

#include "Containers/StringConv.h"
#include "ArraySerializer.h"
#include "Templates/UniquePtr.h"

//region FString

namespace rd {

    FString Polymorphic<FString, void>::read(SerializationCtx& ctx, Buffer& buffer) {
        return FString(std::move(buffer.read_wstring()).data());
    }

    void Polymorphic<FString, void>::write(SerializationCtx& ctx, Buffer& buffer, FString const& value) {
        buffer.write_wstring(wstring_view(GetData(value), value.Len()));
    }


    size_t hash<FString>::operator()(const FString& value) const noexcept {
        return GetTypeHash(value);
    }


}

template class rd::Polymorphic<FString>;
template class rd::Polymorphic<rd::Wrapper<FString>>;
template struct rd::hash<FString>;
// template class rd::Polymorphic<TArray<FString>, void>;

//endregion
