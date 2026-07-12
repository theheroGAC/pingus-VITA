// src/util/i18n.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Internationalization system for Pingus - supports JSON translations
// Works on all platforms including PSVita

#ifndef HEADER_PINGUS_UTIL_I18N_HPP
#define HEADER_PINGUS_UTIL_I18N_HPP

#include <string>
#include <map>
#include <vector>

namespace i18n {

/** Initialize the translation system and load the specified language */
void init(const std::string& language = "en");

/** Get translated string. If not found, returns original msgid */
std::string _(const std::string& msgid);

/** Get plural form. If singular/plural not found, returns msgid_plural */
std::string ngettext(const std::string& msgid_singular, const std::string& msgid_plural, int n);

/** Set current language and reload translations */
void set_language(const std::string& language);

/** Get current language code */
std::string get_language();

/** Get list of available languages */
std::vector<std::string> get_available_languages();

} // namespace i18n

// Convenience macros
#define _(msgid) ::i18n::_(msgid)
#define ngettext(singular, plural, n) ::i18n::ngettext(singular, plural, n)

#endif
