// src/util/i18n.cpp
// SPDX-License-Identifier: GPL-3.0-or-later

#include "util/i18n.hpp"
#include "util/log.hpp"
#include "util/pathname.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

// Undefine convenience macros so they don't interfere with function definitions
#undef _
#undef ngettext

using pingus::Pathname;

namespace i18n {

namespace {
  std::string g_current_language = "en";
  std::map<std::string, std::string> g_translations;
  std::vector<std::string> g_available_languages;
}

// Robust JSON parser for translation files (handles both minified/compressed and pretty-printed formats)
// Format: { "msgid1": "translation1", "msgid2": "translation2" }
static std::map<std::string, std::string> load_json_translations(const Pathname& filepath)
{
  std::map<std::string, std::string> translations;
  std::ifstream file(filepath.get_sys_path());
  
  if (!file.is_open()) {
    log_warn("Could not open translation file: {}", filepath.str());
    return translations;
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  size_t pos = 0;
  size_t length = content.length();
  
  auto unescape = [](std::string& s) {
    size_t p = 0;
    while ((p = s.find("\\n", p)) != std::string::npos) {
      s.replace(p, 2, "\n");
    }
    p = 0;
    while ((p = s.find("\\t", p)) != std::string::npos) {
      s.replace(p, 2, "\t");
    }
    p = 0;
    while ((p = s.find("\\\"", p)) != std::string::npos) {
      s.replace(p, 2, "\"");
    }
    p = 0;
    while ((p = s.find("\\\\", p)) != std::string::npos) {
      s.replace(p, 2, "\\");
    }
  };

  while (pos < length) {
    // Find next double quote for key
    pos = content.find('"', pos);
    if (pos == std::string::npos) break;
    size_t key_start = pos + 1;
    
    // Find closing double quote for key
    pos = content.find('"', key_start);
    if (pos == std::string::npos) break;
    std::string key = content.substr(key_start, pos - key_start);
    pos++; // move past closing quote
    
    // Find colon
    pos = content.find(':', pos);
    if (pos == std::string::npos) break;
    pos++; // move past colon
    
    // Find opening double quote for value
    pos = content.find('"', pos);
    if (pos == std::string::npos) break;
    size_t val_start = pos + 1;
    
    // Find closing double quote for value (must handle escaped quotes \")
    size_t val_end = val_start;
    while (val_end < length) {
      val_end = content.find('"', val_end);
      if (val_end == std::string::npos) break;
      // Check if it's escaped
      if (content[val_end - 1] == '\\') {
        // Count backslashes to see if it's actually escaped
        size_t bs_count = 0;
        size_t bs_pos = val_end - 1;
        while (bs_pos > 0 && content[bs_pos] == '\\') {
          bs_count++;
          bs_pos--;
        }
        if (bs_count % 2 != 0) {
          // Odd number of backslashes, so the quote is escaped
          val_end++; // continue search
          continue;
        }
      }
      break; // found unescaped quote
    }
    if (val_end == std::string::npos || val_end >= length) break;
    
    std::string value = content.substr(val_start, val_end - val_start);
    pos = val_end + 1; // move past closing quote
    
    unescape(key);
    unescape(value);
    
    translations[key] = value;
  }

  return translations;
}

// Discover available languages by checking translation files in data/translations/
static std::vector<std::string> discover_languages()
{
  std::vector<std::string> languages;
  languages.push_back("en"); // English always available

  // Try to find translation files in translations/
  Pathname translations_dir("translations");
  
  // Try common language files
  std::vector<std::string> lang_codes = {
    "de", "es", "fr", "it", "ja", "pt", "ru", "zh",
    "pt_BR", "zh_CN", "zh_TW", "ast", "bg", "cs", "da",
    "fi", "hu", "nb", "nn", "nl", "pl", "sq", "sr", "sv", "th", "tr", "uk"
  };

  for (const auto& lang : lang_codes) {
    Pathname lang_file("translations/" + lang + ".json");
    std::ifstream file(lang_file.get_sys_path());
    if (file.good()) {
      languages.push_back(lang);
    }
  }

  return languages;
}

void init(const std::string& language)
{
  g_current_language = language;
  g_available_languages = discover_languages();
  
  // Try to load the requested language
  Pathname translations_file("translations/" + language + ".json");
  std::string filepath = translations_file.get_sys_path();
  std::ifstream file(filepath);
  
  if (file.good()) {
    g_translations = load_json_translations(translations_file);
    log_info("Loaded translations for language: {} from {}", language, filepath);
  } else {
    log_info("Translations for language '{}' not available (tried {}), using English", language, filepath);
    g_translations.clear();
    g_current_language = "en";
  }
}

std::string _(const std::string& msgid)
{
  // If translation not found, return original
  auto it = g_translations.find(msgid);
  if (it != g_translations.end()) {
    return it->second;
  }
  return msgid;
}

std::string ngettext(const std::string& msgid_singular, const std::string& msgid_plural, int n)
{
  // For now, simple implementation: use plural if n != 1
  if (n == 1) {
    return _(msgid_singular);
  } else {
    return _(msgid_plural);
  }
}

void set_language(const std::string& language)
{
  if (language != g_current_language) {
    init(language);
  }
}

std::string get_language()
{
  return g_current_language;
}

std::vector<std::string> get_available_languages()
{
  return g_available_languages;
}

} // namespace i18n
