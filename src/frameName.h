/*
 * Copyright 2017 Andrei Pangin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FRAMENAME_H
#define _FRAMENAME_H

#include <jvmti.h>
#include <locale.h>
#include <map>
#include <vector>
#include <string>
#include "arguments.h"
#include "mutex.h"
#include "vmEntry.h"

#ifdef __APPLE__
#  include <xlocale.h>
#endif


typedef std::map<jmethodID, std::string> JMethodCache;
typedef std::map<int, std::string> ThreadMap;
typedef std::map<unsigned int, const char*> ClassMap;


enum MatchType {
  MATCH_EQUALS,
  MATCH_CONTAINS,
  MATCH_STARTS_WITH,
  MATCH_ENDS_WITH
};


class Matcher {
  private:
    MatchType _type;
    char* _pattern;
    int _len;

  public:
    Matcher(const char* pattern);
    ~Matcher();

    Matcher(const Matcher& m);
    Matcher& operator=(const Matcher& m);

    bool matches(const char* s);
};


class FrameName {
  private:
    static JMethodCache _cache;

    ClassMap _class_names;
    std::vector<Matcher> _include;
    std::vector<Matcher> _exclude;
    std::string _str;
    int _style;
    unsigned char _cache_epoch;
    unsigned char _cache_max_age;
    Mutex& _thread_names_lock;
    ThreadMap& _thread_names;
    locale_t _saved_locale;
    bool _includemm;
    const int access_flags_size = 12;
    // Based on: https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#:~:text=Table%C2%A04.5.%C2%A0Method%20access%20and%20property%20flags
    // Good practice order from: https://checkstyle.sourceforge.io/config_modifier.html#ModifierOrder
    const std::pair<int, std::string> access_flags [12] = {
        std::make_pair(0x0001, "public"),
        std::make_pair(0x0002, "private"),
        std::make_pair(0x0004, "protected"),
        std::make_pair(0x0400, "abstract"),
        std::make_pair(0x0008, "static"),
        std::make_pair(0x0010, "final"),
        std::make_pair(0x0020, "synchronized"),
        std::make_pair(0x0100, "native"),
        std::make_pair(0x0800, "strict"),
        std::make_pair(0x0040, "bridge"),
        std::make_pair(0x0080, "varargs"),
        std::make_pair(0x1000, "synthetic"),
    };

    void buildFilter(std::vector<Matcher>& vector, const char* base, int offset);
    const char* decodeNativeSymbol(const char* name);
    const char* typeSuffix(FrameTypeId type);
    void javaMethodName(jmethodID method);
    void javaClassName(const char* symbol, size_t length, int style);

  public:
    FrameName(Arguments& args, int style, int epoch, Mutex& thread_names_lock, ThreadMap& thread_names);
    ~FrameName();

    const char* name(ASGCT_CallFrame& frame, bool for_matching = false);

    bool hasIncludeList() { return !_include.empty(); }
    bool hasExcludeList() { return !_exclude.empty(); }

    bool include(const char* frame_name);
    bool exclude(const char* frame_name);
};

#endif // _FRAMENAME_H
