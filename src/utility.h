#pragma once

#include <tl/expected.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace shadercompile
{
template<class F>
class FinalAction
{
public:
    explicit FinalAction(F action) noexcept
        : action_(std::move(action))
        , invoke_(true)
    {}

    FinalAction(FinalAction&& other) noexcept
        : action_(std::move(other.action_))
        , invoke_(other.invoke_)
    {
        other.invoke_ = false;
    }

    FinalAction(FinalAction const&) = delete;

    virtual ~FinalAction() noexcept
    {
        if(invoke_) action_();
    }

    FinalAction& operator=(FinalAction const&) = delete;
    FinalAction& operator=(FinalAction&&) = delete;

protected:
    void dismiss() noexcept { invoke_ = false; }

private:
    F action_;
    bool invoke_;
};

template<class F>
[[nodiscard]] inline FinalAction<F> finally(F const& action) noexcept
{
    return FinalAction<F>(action);
}

template<class F>
[[nodiscard]] inline FinalAction<F> finally(F&& action) noexcept
{
    return FinalAction<F>(std::forward<F>(action));
}

template<class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

void utf8Encode(std::wstring_view wideStr, std::string& outUtf8Str);

std::string utf8Encode(std::wstring_view str);

void utf8Decode(std::string_view utf8Str, std::wstring& outWideStr);

std::wstring utf8Decode(std::string_view utf8Str);

template<class CharT>
struct CaseInsensitiveCharTraits : public std::char_traits<CharT>
{
    [[nodiscard]] static CharT to_upper(CharT ch) { return static_cast<CharT>(std::toupper(ch)); }

    [[nodiscard]] static bool eq(CharT c1, CharT c2) { return to_upper(c1) == to_upper(c2); }

    [[nodiscard]] static bool lt(CharT c1, CharT c2) { return to_upper(c1) < to_upper(c2); }

    [[nodiscard]] static int compare(const CharT* s1, const CharT* s2, size_t n)
    {
        while(n-- != 0)
        {
            if(to_upper(*s1) < to_upper(*s2)) return -1;
            if(to_upper(*s1) > to_upper(*s2)) return 1;
            ++s1;
            ++s2;
        }
        return 0;
    }

    [[nodiscard]] static const CharT* find(const CharT* s, size_t n, CharT a)
    {
        auto const ua(to_upper(a));
        while(n-- != 0)
        {
            if(to_upper(*s) == ua) return s;
            s++;
        }
        return nullptr;
    }
};

template<class CharT, class CharTraitsT, class CharTraitsU>
[[nodiscard]] bool caseInsensitiveEqual(std::basic_string_view<CharT, CharTraitsT> left,
                                        std::basic_string_view<CharT, CharTraitsU> right)
{
    return std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(left.data(), left.size()) ==
           std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(right.data(), right.size());
}

template<class CharT, class CharTraitsT, size_t N>
[[nodiscard]] bool caseInsensitiveEqual(std::basic_string_view<CharT, CharTraitsT> left, const CharT (&right)[N])
{
    return std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(left.data(), left.size()) ==
           std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(right, N - 1);
}

template<class CharT, size_t N, class CharTraitsU>
[[nodiscard]] bool caseInsensitiveEqual(const CharT (&left)[N], std::basic_string_view<CharT, CharTraitsU> right)
{
    return std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(left, N - 1) ==
           std::basic_string_view<CharT, CaseInsensitiveCharTraits<CharT>>(right.data(), right.size());
}

template<std::output_iterator InsertIteratorT, class CharT, class CharTraitsT>
void makeQuotedString(InsertIteratorT insertItr, std::basic_string_view<CharT, CharTraitsT> str)
{
    if(!str.starts_with('"')) { insertItr++ = '"'; }

    std::copy(str.cbegin(), str.cend(), insertItr);

    if(!str.ends_with('"')) { insertItr++ = '"'; }
}

template<class CharT, class CharTraitsT, class AllocT = std::allocator<CharT>>
[[nodiscard]] std::basic_string<CharT, CharTraitsT, AllocT>
makeQuotedString(std::basic_string_view<CharT, CharTraitsT> str)
{
    std::basic_string<CharT, CharTraitsT, AllocT> outStr;
    outStr.reserve(str.size() + 2);

    makeQuotedString(std::back_inserter(outStr));

    return outStr;
}

[[nodiscard]] tl::expected<std::filesystem::path, std::errc>
createTemporaryFilePath(std::wstring_view prefix, std::wstring_view suffix, std::wstring_view extension);

template<class EnumT, std::invocable<EnumT> F>
    requires std::is_enum_v<EnumT>
void forEachEnum(F func)
{
    for(EnumT value = EnumT::_first; value < EnumT::_count;
        value = static_cast<EnumT>(std::underlying_type_t<EnumT>(value) + 1))
    {
        func(value);
    }
}

template<class EnumT, class ContainerT, std::invocable<EnumT, typename ContainerT::value_type&> F>
    requires std::is_enum_v<EnumT>
void forEachEnum(ContainerT& container, F func)
{
    for(EnumT value = EnumT::_first; value < EnumT::_count;
        value = static_cast<EnumT>(std::underlying_type_t<EnumT>(value) + 1))
    {
        func(value, container[(size_t)value]);
    }
}

template<class EnumT, class ContainerT, std::invocable<EnumT, const typename ContainerT::value_type&> F>
    requires std::is_enum_v<EnumT>
void forEachEnum(const ContainerT& container, F func)
{
    for(EnumT value = EnumT::_first; value < EnumT::_count;
        value = static_cast<EnumT>(std::underlying_type_t<EnumT>(value) + 1))
    {
        func(value, container[(size_t)value]);
    }
}

} // namespace shadercompile