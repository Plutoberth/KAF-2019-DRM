#pragma once
#include <utility>

/*
ISC License

Copyright (c) 2018, Mapbox

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

// Use different constants for 32 bit vs. 64 bit size_t
constexpr std::size_t hash_offset =
std::conditional_t < sizeof(std::size_t) < 8,
	std::integral_constant<std::size_t, 0x811C9DC5>,
	std::integral_constant<std::size_t, 0xCBF29CE484222325>>::value;
constexpr std::size_t hash_prime =
std::conditional_t < sizeof(std::size_t) < 8,
	std::integral_constant<std::size_t, 0x1000193>,
	std::integral_constant<std::size_t, 0x100000001B3>>::value;

// FNV-1a hash
constexpr static std::size_t str_hash(const char* str,
	const std::size_t value = hash_offset) noexcept {
	return *str ? str_hash(str + 1, (value ^ static_cast<std::size_t>(*str))* hash_prime) : value;
}

constexpr std::size_t operator"" _hash(char const* s, std::size_t count)
{
	return str_hash(s, count);
}
