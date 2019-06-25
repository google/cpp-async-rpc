/// \file
/// \brief Preprocessor-based iteration and code generation.
///
/// \copyright
///   Copyright 2019 by Google LLC.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef ARPC_PREPROCESSOR_H_
#define ARPC_PREPROCESSOR_H_

#define ARPC_MATCH_ARG_N(                                                      \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
    _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, \
    _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, \
    _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, \
    _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, \
    _92, _93, _94, _95, _96, _97, _98, _99, _100, _101, _102, _103, _104,      \
    _105, _106, _107, _108, _109, _110, _111, _112, _113, _114, _115, _116,    \
    _117, _118, _119, _120, _121, _122, _123, _124, _125, _126, _127, _128,    \
    _129, _130, _131, _132, _133, _134, _135, _136, _137, _138, _139, _140,    \
    _141, _142, _143, _144, _145, _146, _147, _148, _149, _150, _151, _152,    \
    _153, _154, _155, _156, _157, _158, _159, _160, _161, _162, _163, _164,    \
    _165, _166, _167, _168, _169, _170, _171, _172, _173, _174, _175, _176,    \
    _177, _178, _179, _180, _181, _182, _183, _184, _185, _186, _187, _188,    \
    _189, _190, _191, _192, _193, _194, _195, _196, _197, _198, _199, _200,    \
    _201, _202, _203, _204, _205, _206, _207, _208, _209, _210, _211, _212,    \
    _213, _214, _215, _216, _217, _218, _219, _220, _221, _222, _223, _224,    \
    _225, _226, _227, _228, _229, _230, _231, _232, _233, _234, _235, _236,    \
    _237, _238, _239, _240, _241, _242, _243, _244, _245, _246, _247, _248,    \
    _249, _250, _251, _252, _253, _254, _255, _256, n, ...)                    \
  n
#define ARPC_REV_N_SEQ()                                                      \
  256, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242,  \
      241, 240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228,   \
      227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214,   \
      213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200,   \
      199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186,   \
      185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172,   \
      171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158,   \
      157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,   \
      143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130,   \
      129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116,   \
      115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102,   \
      101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85,   \
      84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, \
      66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
      48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
      30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, \
      12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define ARPC_COMMA_PAD_SEQ()                                                  \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0

#define ARPC_HAS_COMMA(...) \
  ARPC_HAS_COMMA_HELPER(__VA_ARGS__, ARPC_COMMA_PAD_SEQ())
#define ARPC_HAS_COMMA_HELPER(...) ARPC_MATCH_ARG_N(__VA_ARGS__)

#define ARPC_TRIGGER_PARENTHESIS(...) ,
#define ARPC_IS_EMPTY(...)                                                   \
  ARPC_IS_EMPTY_HELPER(ARPC_HAS_COMMA(__VA_ARGS__),                          \
                       ARPC_HAS_COMMA(ARPC_TRIGGER_PARENTHESIS __VA_ARGS__), \
                       ARPC_HAS_COMMA(__VA_ARGS__()),                        \
                       ARPC_HAS_COMMA(ARPC_TRIGGER_PARENTHESIS __VA_ARGS__()))
#define ARPC_IS_EMPTY_CASE_0001 ,
#define ARPC_PASTE_5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define ARPC_IS_EMPTY_HELPER(_0, _1, _2, _3) \
  ARPC_HAS_COMMA(ARPC_PASTE_5(ARPC_IS_EMPTY_CASE_, _0, _1, _2, _3))

// Concatenate two symbols.
#define ARPC_CONCAT(a, b) a##b

// Conditionals.
#define ARPC_SECOND(a, b, ...) b
#define ARPC_IS_PROBE(...) ARPC_SECOND(__VA_ARGS__, 0)
#define ARPC_PROBE() ~, 1

#define ARPC_NOT(x) ARPC_IS_PROBE(ARPC_CONCAT(ARPC_NOT_HELPER_, x))
#define ARPC_NOT_HELPER_0 ARPC_PROBE()
#define ARPC_BOOL(x) ARPC_NOT(ARPC_NOT(x))

#define ARPC_IF(condition) ARPC_IF_HELPER(ARPC_BOOL(condition))
#define ARPC_IF_HELPER(condition) ARPC_CONCAT(ARPC_IF_THEN_, condition)
#define ARPC_IF_THEN_0(...)
#define ARPC_IF_THEN_1(...) __VA_ARGS__

#define ARPC_IF_ELSE(condition) ARPC_IF_ELSE_HELPER(ARPC_BOOL(condition))
#define ARPC_IF_ELSE_HELPER(condition) \
  ARPC_CONCAT(ARPC_IF_THEN_ELSE_, condition)
#define ARPC_IF_THEN_ELSE_0(...) ARPC_IF_ELSE_0
#define ARPC_IF_THEN_ELSE_1(...) __VA_ARGS__ ARPC_IF_ELSE_1
#define ARPC_IF_ELSE_0(...) __VA_ARGS__
#define ARPC_IF_ELSE_1(...)

// Convert to a string.
#define ARPC_STRINGIZE(s) #s

// Expand to nothing.
#define ARPC_EMPTY()

// Remove parenthesis.
#define ARPC_NO_PARENS(MACRO, ...) MACRO __VA_ARGS__

// Defer expansion.
#define ARPC_DEFER_1(MACRO) MACRO ARPC_EMPTY()
#define ARPC_DEFER_2(MACRO) MACRO ARPC_EMPTY ARPC_EMPTY()()
#define ARPC_DEFER_3(MACRO) MACRO ARPC_EMPTY ARPC_EMPTY ARPC_EMPTY()()()
#define ARPC_DEFER_4(MACRO) \
  MACRO ARPC_EMPTY ARPC_EMPTY ARPC_EMPTY ARPC_EMPTY()()()()

// Force recursive expansion.
#define ARPC_EXPAND_EMPTY
#define ARPC_EXPAND(...) ARPC_EXPAND_1024(__VA_ARGS__)
#define ARPC_EXPAND_1024(...) ARPC_EXPAND_512(ARPC_EXPAND_512(__VA_ARGS__))
#define ARPC_EXPAND_512(...) ARPC_EXPAND_256(ARPC_EXPAND_256(__VA_ARGS__))
#define ARPC_EXPAND_256(...) ARPC_EXPAND_128(ARPC_EXPAND_128(__VA_ARGS__))
#define ARPC_EXPAND_128(...) ARPC_EXPAND_64(ARPC_EXPAND_64(__VA_ARGS__))
#define ARPC_EXPAND_64(...) ARPC_EXPAND_32(ARPC_EXPAND_32(__VA_ARGS__))
#define ARPC_EXPAND_32(...) ARPC_EXPAND_16(ARPC_EXPAND_16(__VA_ARGS__))
#define ARPC_EXPAND_16(...) ARPC_EXPAND_8(ARPC_EXPAND_8(__VA_ARGS__))
#define ARPC_EXPAND_8(...) ARPC_EXPAND_4(ARPC_EXPAND_4(__VA_ARGS__))
#define ARPC_EXPAND_4(...) ARPC_EXPAND_2(ARPC_EXPAND_2(__VA_ARGS__))
#define ARPC_EXPAND_2(...) ARPC_EXPAND_1(ARPC_EXPAND_1(__VA_ARGS__))
#define ARPC_EXPAND_1(...) __VA_ARGS__

/// Count the number of arguments (actually commas, as no arguments counts as
/// one) in the variadic args.
#define ARPC_ARG_COUNT(...)                \
  ARPC_IF_ELSE(ARPC_IS_EMPTY(__VA_ARGS__)) \
  (0)(ARPC_ARG_COUNT_HELPER(__VA_ARGS__, ARPC_REV_N_SEQ()))
#define ARPC_ARG_COUNT_HELPER(...) ARPC_MATCH_ARG_N(__VA_ARGS__)

// Expand what(x) for every x in the variadic args, with sep() between each two.
#define ARPC_FOREACH(what, sep, ...) \
  ARPC_FOREACH_N(ARPC_ARG_COUNT(__VA_ARGS__), what, sep, __VA_ARGS__)
#define ARPC_FOREACH_EXPAND(...) __VA_ARGS__
#define ARPC_FOREACH_N(n, what, sep, ...) \
  ARPC_FOREACH_EXPAND(ARPC_CONCAT(ARPC_FOREACH_, n)(what, sep, __VA_ARGS__))

// For deferred invocation:
#define ARPC_FOREACH_AGAIN() ARPC_FOREACH

// clang-format off
#define ARPC_FOREACH_0(what, sep, ...)
#define ARPC_FOREACH_1(what, sep, first, ...) what(first)
#define ARPC_FOREACH_2(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_1(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_3(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_2(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_4(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_3(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_5(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_4(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_6(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_5(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_7(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_6(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_8(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_7(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_9(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_8(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_10(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_9(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_11(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_10(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_12(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_11(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_13(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_12(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_14(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_13(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_15(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_14(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_16(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_15(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_17(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_16(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_18(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_17(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_19(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_18(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_20(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_19(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_21(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_20(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_22(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_21(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_23(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_22(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_24(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_23(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_25(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_24(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_26(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_25(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_27(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_26(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_28(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_27(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_29(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_28(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_30(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_29(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_31(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_30(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_32(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_31(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_33(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_32(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_34(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_33(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_35(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_34(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_36(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_35(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_37(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_36(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_38(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_37(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_39(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_38(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_40(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_39(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_41(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_40(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_42(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_41(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_43(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_42(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_44(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_43(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_45(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_44(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_46(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_45(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_47(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_46(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_48(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_47(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_49(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_48(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_50(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_49(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_51(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_50(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_52(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_51(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_53(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_52(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_54(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_53(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_55(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_54(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_56(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_55(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_57(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_56(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_58(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_57(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_59(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_58(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_60(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_59(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_61(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_60(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_62(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_61(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_63(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_62(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_64(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_63(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_65(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_64(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_66(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_65(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_67(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_66(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_68(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_67(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_69(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_68(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_70(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_69(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_71(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_70(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_72(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_71(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_73(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_72(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_74(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_73(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_75(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_74(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_76(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_75(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_77(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_76(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_78(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_77(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_79(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_78(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_80(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_79(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_81(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_80(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_82(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_81(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_83(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_82(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_84(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_83(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_85(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_84(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_86(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_85(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_87(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_86(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_88(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_87(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_89(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_88(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_90(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_89(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_91(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_90(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_92(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_91(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_93(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_92(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_94(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_93(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_95(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_94(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_96(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_95(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_97(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_96(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_98(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_97(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_99(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_98(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_100(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_99(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_101(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_100(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_102(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_101(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_103(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_102(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_104(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_103(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_105(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_104(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_106(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_105(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_107(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_106(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_108(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_107(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_109(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_108(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_110(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_109(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_111(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_110(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_112(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_111(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_113(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_112(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_114(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_113(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_115(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_114(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_116(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_115(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_117(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_116(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_118(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_117(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_119(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_118(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_120(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_119(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_121(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_120(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_122(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_121(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_123(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_122(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_124(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_123(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_125(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_124(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_126(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_125(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_127(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_126(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_128(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_127(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_129(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_128(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_130(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_129(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_131(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_130(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_132(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_131(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_133(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_132(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_134(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_133(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_135(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_134(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_136(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_135(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_137(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_136(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_138(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_137(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_139(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_138(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_140(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_139(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_141(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_140(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_142(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_141(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_143(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_142(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_144(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_143(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_145(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_144(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_146(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_145(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_147(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_146(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_148(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_147(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_149(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_148(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_150(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_149(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_151(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_150(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_152(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_151(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_153(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_152(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_154(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_153(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_155(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_154(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_156(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_155(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_157(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_156(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_158(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_157(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_159(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_158(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_160(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_159(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_161(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_160(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_162(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_161(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_163(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_162(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_164(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_163(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_165(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_164(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_166(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_165(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_167(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_166(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_168(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_167(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_169(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_168(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_170(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_169(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_171(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_170(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_172(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_171(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_173(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_172(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_174(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_173(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_175(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_174(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_176(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_175(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_177(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_176(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_178(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_177(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_179(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_178(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_180(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_179(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_181(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_180(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_182(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_181(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_183(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_182(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_184(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_183(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_185(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_184(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_186(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_185(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_187(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_186(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_188(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_187(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_189(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_188(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_190(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_189(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_191(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_190(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_192(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_191(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_193(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_192(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_194(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_193(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_195(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_194(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_196(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_195(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_197(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_196(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_198(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_197(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_199(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_198(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_200(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_199(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_201(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_200(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_202(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_201(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_203(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_202(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_204(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_203(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_205(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_204(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_206(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_205(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_207(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_206(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_208(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_207(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_209(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_208(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_210(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_209(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_211(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_210(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_212(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_211(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_213(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_212(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_214(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_213(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_215(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_214(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_216(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_215(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_217(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_216(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_218(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_217(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_219(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_218(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_220(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_219(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_221(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_220(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_222(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_221(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_223(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_222(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_224(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_223(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_225(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_224(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_226(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_225(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_227(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_226(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_228(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_227(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_229(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_228(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_230(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_229(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_231(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_230(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_232(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_231(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_233(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_232(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_234(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_233(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_235(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_234(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_236(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_235(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_237(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_236(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_238(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_237(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_239(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_238(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_240(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_239(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_241(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_240(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_242(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_241(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_243(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_242(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_244(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_243(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_245(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_244(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_246(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_245(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_247(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_246(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_248(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_247(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_249(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_248(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_250(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_249(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_251(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_250(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_252(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_251(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_253(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_252(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_254(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_253(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_255(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_254(what, sep, __VA_ARGS__)
#define ARPC_FOREACH_256(what, sep, first, ...) \
  what(first)sep()ARPC_FOREACH_255(what, sep, __VA_ARGS__)
// clang-format on

// Expand what(i, __VA_ARGS__) sep()? for every i counting down from count
// to 1, both inclusive. Count must be an integer literal from 0 to 256.
#define ARPC_FOR(count, what, sep, ...) \
  ARPC_FOR_##count(begin, end, what, sep, __VA_ARGS__)

// For deferred invocation:
#define ARPC_FOR_AGAIN() ARPC_FOR

// clang-format off
#define ARPC_FOR_0(begin, end, what, sep, ...)
#define ARPC_FOR_1(begin, end, what, sep, ...) \
  what(1, __VA_ARGS__)
#define ARPC_FOR_2(begin, end, what, sep, ...) \
  what(2, __VA_ARGS__)sep()ARPC_FOR_1(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_3(begin, end, what, sep, ...) \
  what(3, __VA_ARGS__)sep()ARPC_FOR_2(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_4(begin, end, what, sep, ...) \
  what(4, __VA_ARGS__)sep()ARPC_FOR_3(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_5(begin, end, what, sep, ...) \
  what(5, __VA_ARGS__)sep()ARPC_FOR_4(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_6(begin, end, what, sep, ...) \
  what(6, __VA_ARGS__)sep()ARPC_FOR_5(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_7(begin, end, what, sep, ...) \
  what(7, __VA_ARGS__)sep()ARPC_FOR_6(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_8(begin, end, what, sep, ...) \
  what(8, __VA_ARGS__)sep()ARPC_FOR_7(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_9(begin, end, what, sep, ...) \
  what(9, __VA_ARGS__)sep()ARPC_FOR_8(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_10(begin, end, what, sep, ...) \
  what(10, __VA_ARGS__)sep()ARPC_FOR_9(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_11(begin, end, what, sep, ...) \
  what(11, __VA_ARGS__)sep()ARPC_FOR_10(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_12(begin, end, what, sep, ...) \
  what(12, __VA_ARGS__)sep()ARPC_FOR_11(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_13(begin, end, what, sep, ...) \
  what(13, __VA_ARGS__)sep()ARPC_FOR_12(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_14(begin, end, what, sep, ...) \
  what(14, __VA_ARGS__)sep()ARPC_FOR_13(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_15(begin, end, what, sep, ...) \
  what(15, __VA_ARGS__)sep()ARPC_FOR_14(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_16(begin, end, what, sep, ...) \
  what(16, __VA_ARGS__)sep()ARPC_FOR_15(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_17(begin, end, what, sep, ...) \
  what(17, __VA_ARGS__)sep()ARPC_FOR_16(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_18(begin, end, what, sep, ...) \
  what(18, __VA_ARGS__)sep()ARPC_FOR_17(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_19(begin, end, what, sep, ...) \
  what(19, __VA_ARGS__)sep()ARPC_FOR_18(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_20(begin, end, what, sep, ...) \
  what(20, __VA_ARGS__)sep()ARPC_FOR_19(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_21(begin, end, what, sep, ...) \
  what(21, __VA_ARGS__)sep()ARPC_FOR_20(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_22(begin, end, what, sep, ...) \
  what(22, __VA_ARGS__)sep()ARPC_FOR_21(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_23(begin, end, what, sep, ...) \
  what(23, __VA_ARGS__)sep()ARPC_FOR_22(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_24(begin, end, what, sep, ...) \
  what(24, __VA_ARGS__)sep()ARPC_FOR_23(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_25(begin, end, what, sep, ...) \
  what(25, __VA_ARGS__)sep()ARPC_FOR_24(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_26(begin, end, what, sep, ...) \
  what(26, __VA_ARGS__)sep()ARPC_FOR_25(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_27(begin, end, what, sep, ...) \
  what(27, __VA_ARGS__)sep()ARPC_FOR_26(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_28(begin, end, what, sep, ...) \
  what(28, __VA_ARGS__)sep()ARPC_FOR_27(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_29(begin, end, what, sep, ...) \
  what(29, __VA_ARGS__)sep()ARPC_FOR_28(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_30(begin, end, what, sep, ...) \
  what(30, __VA_ARGS__)sep()ARPC_FOR_29(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_31(begin, end, what, sep, ...) \
  what(31, __VA_ARGS__)sep()ARPC_FOR_30(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_32(begin, end, what, sep, ...) \
  what(32, __VA_ARGS__)sep()ARPC_FOR_31(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_33(begin, end, what, sep, ...) \
  what(33, __VA_ARGS__)sep()ARPC_FOR_32(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_34(begin, end, what, sep, ...) \
  what(34, __VA_ARGS__)sep()ARPC_FOR_33(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_35(begin, end, what, sep, ...) \
  what(35, __VA_ARGS__)sep()ARPC_FOR_34(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_36(begin, end, what, sep, ...) \
  what(36, __VA_ARGS__)sep()ARPC_FOR_35(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_37(begin, end, what, sep, ...) \
  what(37, __VA_ARGS__)sep()ARPC_FOR_36(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_38(begin, end, what, sep, ...) \
  what(38, __VA_ARGS__)sep()ARPC_FOR_37(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_39(begin, end, what, sep, ...) \
  what(39, __VA_ARGS__)sep()ARPC_FOR_38(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_40(begin, end, what, sep, ...) \
  what(40, __VA_ARGS__)sep()ARPC_FOR_39(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_41(begin, end, what, sep, ...) \
  what(41, __VA_ARGS__)sep()ARPC_FOR_40(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_42(begin, end, what, sep, ...) \
  what(42, __VA_ARGS__)sep()ARPC_FOR_41(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_43(begin, end, what, sep, ...) \
  what(43, __VA_ARGS__)sep()ARPC_FOR_42(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_44(begin, end, what, sep, ...) \
  what(44, __VA_ARGS__)sep()ARPC_FOR_43(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_45(begin, end, what, sep, ...) \
  what(45, __VA_ARGS__)sep()ARPC_FOR_44(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_46(begin, end, what, sep, ...) \
  what(46, __VA_ARGS__)sep()ARPC_FOR_45(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_47(begin, end, what, sep, ...) \
  what(47, __VA_ARGS__)sep()ARPC_FOR_46(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_48(begin, end, what, sep, ...) \
  what(48, __VA_ARGS__)sep()ARPC_FOR_47(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_49(begin, end, what, sep, ...) \
  what(49, __VA_ARGS__)sep()ARPC_FOR_48(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_50(begin, end, what, sep, ...) \
  what(50, __VA_ARGS__)sep()ARPC_FOR_49(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_51(begin, end, what, sep, ...) \
  what(51, __VA_ARGS__)sep()ARPC_FOR_50(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_52(begin, end, what, sep, ...) \
  what(52, __VA_ARGS__)sep()ARPC_FOR_51(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_53(begin, end, what, sep, ...) \
  what(53, __VA_ARGS__)sep()ARPC_FOR_52(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_54(begin, end, what, sep, ...) \
  what(54, __VA_ARGS__)sep()ARPC_FOR_53(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_55(begin, end, what, sep, ...) \
  what(55, __VA_ARGS__)sep()ARPC_FOR_54(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_56(begin, end, what, sep, ...) \
  what(56, __VA_ARGS__)sep()ARPC_FOR_55(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_57(begin, end, what, sep, ...) \
  what(57, __VA_ARGS__)sep()ARPC_FOR_56(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_58(begin, end, what, sep, ...) \
  what(58, __VA_ARGS__)sep()ARPC_FOR_57(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_59(begin, end, what, sep, ...) \
  what(59, __VA_ARGS__)sep()ARPC_FOR_58(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_60(begin, end, what, sep, ...) \
  what(60, __VA_ARGS__)sep()ARPC_FOR_59(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_61(begin, end, what, sep, ...) \
  what(61, __VA_ARGS__)sep()ARPC_FOR_60(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_62(begin, end, what, sep, ...) \
  what(62, __VA_ARGS__)sep()ARPC_FOR_61(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_63(begin, end, what, sep, ...) \
  what(63, __VA_ARGS__)sep()ARPC_FOR_62(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_64(begin, end, what, sep, ...) \
  what(64, __VA_ARGS__)sep()ARPC_FOR_63(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_65(begin, end, what, sep, ...) \
  what(65, __VA_ARGS__)sep()ARPC_FOR_64(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_66(begin, end, what, sep, ...) \
  what(66, __VA_ARGS__)sep()ARPC_FOR_65(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_67(begin, end, what, sep, ...) \
  what(67, __VA_ARGS__)sep()ARPC_FOR_66(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_68(begin, end, what, sep, ...) \
  what(68, __VA_ARGS__)sep()ARPC_FOR_67(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_69(begin, end, what, sep, ...) \
  what(69, __VA_ARGS__)sep()ARPC_FOR_68(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_70(begin, end, what, sep, ...) \
  what(70, __VA_ARGS__)sep()ARPC_FOR_69(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_71(begin, end, what, sep, ...) \
  what(71, __VA_ARGS__)sep()ARPC_FOR_70(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_72(begin, end, what, sep, ...) \
  what(72, __VA_ARGS__)sep()ARPC_FOR_71(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_73(begin, end, what, sep, ...) \
  what(73, __VA_ARGS__)sep()ARPC_FOR_72(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_74(begin, end, what, sep, ...) \
  what(74, __VA_ARGS__)sep()ARPC_FOR_73(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_75(begin, end, what, sep, ...) \
  what(75, __VA_ARGS__)sep()ARPC_FOR_74(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_76(begin, end, what, sep, ...) \
  what(76, __VA_ARGS__)sep()ARPC_FOR_75(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_77(begin, end, what, sep, ...) \
  what(77, __VA_ARGS__)sep()ARPC_FOR_76(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_78(begin, end, what, sep, ...) \
  what(78, __VA_ARGS__)sep()ARPC_FOR_77(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_79(begin, end, what, sep, ...) \
  what(79, __VA_ARGS__)sep()ARPC_FOR_78(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_80(begin, end, what, sep, ...) \
  what(80, __VA_ARGS__)sep()ARPC_FOR_79(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_81(begin, end, what, sep, ...) \
  what(81, __VA_ARGS__)sep()ARPC_FOR_80(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_82(begin, end, what, sep, ...) \
  what(82, __VA_ARGS__)sep()ARPC_FOR_81(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_83(begin, end, what, sep, ...) \
  what(83, __VA_ARGS__)sep()ARPC_FOR_82(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_84(begin, end, what, sep, ...) \
  what(84, __VA_ARGS__)sep()ARPC_FOR_83(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_85(begin, end, what, sep, ...) \
  what(85, __VA_ARGS__)sep()ARPC_FOR_84(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_86(begin, end, what, sep, ...) \
  what(86, __VA_ARGS__)sep()ARPC_FOR_85(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_87(begin, end, what, sep, ...) \
  what(87, __VA_ARGS__)sep()ARPC_FOR_86(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_88(begin, end, what, sep, ...) \
  what(88, __VA_ARGS__)sep()ARPC_FOR_87(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_89(begin, end, what, sep, ...) \
  what(89, __VA_ARGS__)sep()ARPC_FOR_88(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_90(begin, end, what, sep, ...) \
  what(90, __VA_ARGS__)sep()ARPC_FOR_89(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_91(begin, end, what, sep, ...) \
  what(91, __VA_ARGS__)sep()ARPC_FOR_90(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_92(begin, end, what, sep, ...) \
  what(92, __VA_ARGS__)sep()ARPC_FOR_91(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_93(begin, end, what, sep, ...) \
  what(93, __VA_ARGS__)sep()ARPC_FOR_92(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_94(begin, end, what, sep, ...) \
  what(94, __VA_ARGS__)sep()ARPC_FOR_93(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_95(begin, end, what, sep, ...) \
  what(95, __VA_ARGS__)sep()ARPC_FOR_94(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_96(begin, end, what, sep, ...) \
  what(96, __VA_ARGS__)sep()ARPC_FOR_95(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_97(begin, end, what, sep, ...) \
  what(97, __VA_ARGS__)sep()ARPC_FOR_96(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_98(begin, end, what, sep, ...) \
  what(98, __VA_ARGS__)sep()ARPC_FOR_97(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_99(begin, end, what, sep, ...) \
  what(99, __VA_ARGS__)sep()ARPC_FOR_98(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_100(begin, end, what, sep, ...) \
  what(100, __VA_ARGS__)sep()ARPC_FOR_99(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_101(begin, end, what, sep, ...) \
  what(101, __VA_ARGS__)sep()ARPC_FOR_100(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_102(begin, end, what, sep, ...) \
  what(102, __VA_ARGS__)sep()ARPC_FOR_101(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_103(begin, end, what, sep, ...) \
  what(103, __VA_ARGS__)sep()ARPC_FOR_102(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_104(begin, end, what, sep, ...) \
  what(104, __VA_ARGS__)sep()ARPC_FOR_103(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_105(begin, end, what, sep, ...) \
  what(105, __VA_ARGS__)sep()ARPC_FOR_104(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_106(begin, end, what, sep, ...) \
  what(106, __VA_ARGS__)sep()ARPC_FOR_105(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_107(begin, end, what, sep, ...) \
  what(107, __VA_ARGS__)sep()ARPC_FOR_106(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_108(begin, end, what, sep, ...) \
  what(108, __VA_ARGS__)sep()ARPC_FOR_107(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_109(begin, end, what, sep, ...) \
  what(109, __VA_ARGS__)sep()ARPC_FOR_108(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_110(begin, end, what, sep, ...) \
  what(110, __VA_ARGS__)sep()ARPC_FOR_109(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_111(begin, end, what, sep, ...) \
  what(111, __VA_ARGS__)sep()ARPC_FOR_110(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_112(begin, end, what, sep, ...) \
  what(112, __VA_ARGS__)sep()ARPC_FOR_111(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_113(begin, end, what, sep, ...) \
  what(113, __VA_ARGS__)sep()ARPC_FOR_112(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_114(begin, end, what, sep, ...) \
  what(114, __VA_ARGS__)sep()ARPC_FOR_113(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_115(begin, end, what, sep, ...) \
  what(115, __VA_ARGS__)sep()ARPC_FOR_114(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_116(begin, end, what, sep, ...) \
  what(116, __VA_ARGS__)sep()ARPC_FOR_115(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_117(begin, end, what, sep, ...) \
  what(117, __VA_ARGS__)sep()ARPC_FOR_116(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_118(begin, end, what, sep, ...) \
  what(118, __VA_ARGS__)sep()ARPC_FOR_117(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_119(begin, end, what, sep, ...) \
  what(119, __VA_ARGS__)sep()ARPC_FOR_118(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_120(begin, end, what, sep, ...) \
  what(120, __VA_ARGS__)sep()ARPC_FOR_119(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_121(begin, end, what, sep, ...) \
  what(121, __VA_ARGS__)sep()ARPC_FOR_120(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_122(begin, end, what, sep, ...) \
  what(122, __VA_ARGS__)sep()ARPC_FOR_121(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_123(begin, end, what, sep, ...) \
  what(123, __VA_ARGS__)sep()ARPC_FOR_122(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_124(begin, end, what, sep, ...) \
  what(124, __VA_ARGS__)sep()ARPC_FOR_123(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_125(begin, end, what, sep, ...) \
  what(125, __VA_ARGS__)sep()ARPC_FOR_124(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_126(begin, end, what, sep, ...) \
  what(126, __VA_ARGS__)sep()ARPC_FOR_125(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_127(begin, end, what, sep, ...) \
  what(127, __VA_ARGS__)sep()ARPC_FOR_126(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_128(begin, end, what, sep, ...) \
  what(128, __VA_ARGS__)sep()ARPC_FOR_127(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_129(begin, end, what, sep, ...) \
  what(129, __VA_ARGS__)sep()ARPC_FOR_128(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_130(begin, end, what, sep, ...) \
  what(130, __VA_ARGS__)sep()ARPC_FOR_129(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_131(begin, end, what, sep, ...) \
  what(131, __VA_ARGS__)sep()ARPC_FOR_130(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_132(begin, end, what, sep, ...) \
  what(132, __VA_ARGS__)sep()ARPC_FOR_131(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_133(begin, end, what, sep, ...) \
  what(133, __VA_ARGS__)sep()ARPC_FOR_132(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_134(begin, end, what, sep, ...) \
  what(134, __VA_ARGS__)sep()ARPC_FOR_133(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_135(begin, end, what, sep, ...) \
  what(135, __VA_ARGS__)sep()ARPC_FOR_134(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_136(begin, end, what, sep, ...) \
  what(136, __VA_ARGS__)sep()ARPC_FOR_135(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_137(begin, end, what, sep, ...) \
  what(137, __VA_ARGS__)sep()ARPC_FOR_136(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_138(begin, end, what, sep, ...) \
  what(138, __VA_ARGS__)sep()ARPC_FOR_137(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_139(begin, end, what, sep, ...) \
  what(139, __VA_ARGS__)sep()ARPC_FOR_138(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_140(begin, end, what, sep, ...) \
  what(140, __VA_ARGS__)sep()ARPC_FOR_139(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_141(begin, end, what, sep, ...) \
  what(141, __VA_ARGS__)sep()ARPC_FOR_140(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_142(begin, end, what, sep, ...) \
  what(142, __VA_ARGS__)sep()ARPC_FOR_141(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_143(begin, end, what, sep, ...) \
  what(143, __VA_ARGS__)sep()ARPC_FOR_142(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_144(begin, end, what, sep, ...) \
  what(144, __VA_ARGS__)sep()ARPC_FOR_143(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_145(begin, end, what, sep, ...) \
  what(145, __VA_ARGS__)sep()ARPC_FOR_144(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_146(begin, end, what, sep, ...) \
  what(146, __VA_ARGS__)sep()ARPC_FOR_145(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_147(begin, end, what, sep, ...) \
  what(147, __VA_ARGS__)sep()ARPC_FOR_146(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_148(begin, end, what, sep, ...) \
  what(148, __VA_ARGS__)sep()ARPC_FOR_147(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_149(begin, end, what, sep, ...) \
  what(149, __VA_ARGS__)sep()ARPC_FOR_148(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_150(begin, end, what, sep, ...) \
  what(150, __VA_ARGS__)sep()ARPC_FOR_149(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_151(begin, end, what, sep, ...) \
  what(151, __VA_ARGS__)sep()ARPC_FOR_150(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_152(begin, end, what, sep, ...) \
  what(152, __VA_ARGS__)sep()ARPC_FOR_151(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_153(begin, end, what, sep, ...) \
  what(153, __VA_ARGS__)sep()ARPC_FOR_152(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_154(begin, end, what, sep, ...) \
  what(154, __VA_ARGS__)sep()ARPC_FOR_153(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_155(begin, end, what, sep, ...) \
  what(155, __VA_ARGS__)sep()ARPC_FOR_154(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_156(begin, end, what, sep, ...) \
  what(156, __VA_ARGS__)sep()ARPC_FOR_155(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_157(begin, end, what, sep, ...) \
  what(157, __VA_ARGS__)sep()ARPC_FOR_156(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_158(begin, end, what, sep, ...) \
  what(158, __VA_ARGS__)sep()ARPC_FOR_157(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_159(begin, end, what, sep, ...) \
  what(159, __VA_ARGS__)sep()ARPC_FOR_158(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_160(begin, end, what, sep, ...) \
  what(160, __VA_ARGS__)sep()ARPC_FOR_159(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_161(begin, end, what, sep, ...) \
  what(161, __VA_ARGS__)sep()ARPC_FOR_160(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_162(begin, end, what, sep, ...) \
  what(162, __VA_ARGS__)sep()ARPC_FOR_161(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_163(begin, end, what, sep, ...) \
  what(163, __VA_ARGS__)sep()ARPC_FOR_162(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_164(begin, end, what, sep, ...) \
  what(164, __VA_ARGS__)sep()ARPC_FOR_163(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_165(begin, end, what, sep, ...) \
  what(165, __VA_ARGS__)sep()ARPC_FOR_164(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_166(begin, end, what, sep, ...) \
  what(166, __VA_ARGS__)sep()ARPC_FOR_165(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_167(begin, end, what, sep, ...) \
  what(167, __VA_ARGS__)sep()ARPC_FOR_166(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_168(begin, end, what, sep, ...) \
  what(168, __VA_ARGS__)sep()ARPC_FOR_167(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_169(begin, end, what, sep, ...) \
  what(169, __VA_ARGS__)sep()ARPC_FOR_168(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_170(begin, end, what, sep, ...) \
  what(170, __VA_ARGS__)sep()ARPC_FOR_169(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_171(begin, end, what, sep, ...) \
  what(171, __VA_ARGS__)sep()ARPC_FOR_170(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_172(begin, end, what, sep, ...) \
  what(172, __VA_ARGS__)sep()ARPC_FOR_171(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_173(begin, end, what, sep, ...) \
  what(173, __VA_ARGS__)sep()ARPC_FOR_172(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_174(begin, end, what, sep, ...) \
  what(174, __VA_ARGS__)sep()ARPC_FOR_173(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_175(begin, end, what, sep, ...) \
  what(175, __VA_ARGS__)sep()ARPC_FOR_174(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_176(begin, end, what, sep, ...) \
  what(176, __VA_ARGS__)sep()ARPC_FOR_175(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_177(begin, end, what, sep, ...) \
  what(177, __VA_ARGS__)sep()ARPC_FOR_176(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_178(begin, end, what, sep, ...) \
  what(178, __VA_ARGS__)sep()ARPC_FOR_177(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_179(begin, end, what, sep, ...) \
  what(179, __VA_ARGS__)sep()ARPC_FOR_178(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_180(begin, end, what, sep, ...) \
  what(180, __VA_ARGS__)sep()ARPC_FOR_179(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_181(begin, end, what, sep, ...) \
  what(181, __VA_ARGS__)sep()ARPC_FOR_180(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_182(begin, end, what, sep, ...) \
  what(182, __VA_ARGS__)sep()ARPC_FOR_181(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_183(begin, end, what, sep, ...) \
  what(183, __VA_ARGS__)sep()ARPC_FOR_182(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_184(begin, end, what, sep, ...) \
  what(184, __VA_ARGS__)sep()ARPC_FOR_183(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_185(begin, end, what, sep, ...) \
  what(185, __VA_ARGS__)sep()ARPC_FOR_184(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_186(begin, end, what, sep, ...) \
  what(186, __VA_ARGS__)sep()ARPC_FOR_185(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_187(begin, end, what, sep, ...) \
  what(187, __VA_ARGS__)sep()ARPC_FOR_186(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_188(begin, end, what, sep, ...) \
  what(188, __VA_ARGS__)sep()ARPC_FOR_187(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_189(begin, end, what, sep, ...) \
  what(189, __VA_ARGS__)sep()ARPC_FOR_188(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_190(begin, end, what, sep, ...) \
  what(190, __VA_ARGS__)sep()ARPC_FOR_189(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_191(begin, end, what, sep, ...) \
  what(191, __VA_ARGS__)sep()ARPC_FOR_190(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_192(begin, end, what, sep, ...) \
  what(192, __VA_ARGS__)sep()ARPC_FOR_191(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_193(begin, end, what, sep, ...) \
  what(193, __VA_ARGS__)sep()ARPC_FOR_192(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_194(begin, end, what, sep, ...) \
  what(194, __VA_ARGS__)sep()ARPC_FOR_193(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_195(begin, end, what, sep, ...) \
  what(195, __VA_ARGS__)sep()ARPC_FOR_194(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_196(begin, end, what, sep, ...) \
  what(196, __VA_ARGS__)sep()ARPC_FOR_195(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_197(begin, end, what, sep, ...) \
  what(197, __VA_ARGS__)sep()ARPC_FOR_196(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_198(begin, end, what, sep, ...) \
  what(198, __VA_ARGS__)sep()ARPC_FOR_197(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_199(begin, end, what, sep, ...) \
  what(199, __VA_ARGS__)sep()ARPC_FOR_198(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_200(begin, end, what, sep, ...) \
  what(200, __VA_ARGS__)sep()ARPC_FOR_199(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_201(begin, end, what, sep, ...) \
  what(201, __VA_ARGS__)sep()ARPC_FOR_200(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_202(begin, end, what, sep, ...) \
  what(202, __VA_ARGS__)sep()ARPC_FOR_201(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_203(begin, end, what, sep, ...) \
  what(203, __VA_ARGS__)sep()ARPC_FOR_202(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_204(begin, end, what, sep, ...) \
  what(204, __VA_ARGS__)sep()ARPC_FOR_203(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_205(begin, end, what, sep, ...) \
  what(205, __VA_ARGS__)sep()ARPC_FOR_204(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_206(begin, end, what, sep, ...) \
  what(206, __VA_ARGS__)sep()ARPC_FOR_205(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_207(begin, end, what, sep, ...) \
  what(207, __VA_ARGS__)sep()ARPC_FOR_206(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_208(begin, end, what, sep, ...) \
  what(208, __VA_ARGS__)sep()ARPC_FOR_207(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_209(begin, end, what, sep, ...) \
  what(209, __VA_ARGS__)sep()ARPC_FOR_208(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_210(begin, end, what, sep, ...) \
  what(210, __VA_ARGS__)sep()ARPC_FOR_209(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_211(begin, end, what, sep, ...) \
  what(211, __VA_ARGS__)sep()ARPC_FOR_210(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_212(begin, end, what, sep, ...) \
  what(212, __VA_ARGS__)sep()ARPC_FOR_211(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_213(begin, end, what, sep, ...) \
  what(213, __VA_ARGS__)sep()ARPC_FOR_212(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_214(begin, end, what, sep, ...) \
  what(214, __VA_ARGS__)sep()ARPC_FOR_213(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_215(begin, end, what, sep, ...) \
  what(215, __VA_ARGS__)sep()ARPC_FOR_214(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_216(begin, end, what, sep, ...) \
  what(216, __VA_ARGS__)sep()ARPC_FOR_215(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_217(begin, end, what, sep, ...) \
  what(217, __VA_ARGS__)sep()ARPC_FOR_216(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_218(begin, end, what, sep, ...) \
  what(218, __VA_ARGS__)sep()ARPC_FOR_217(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_219(begin, end, what, sep, ...) \
  what(219, __VA_ARGS__)sep()ARPC_FOR_218(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_220(begin, end, what, sep, ...) \
  what(220, __VA_ARGS__)sep()ARPC_FOR_219(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_221(begin, end, what, sep, ...) \
  what(221, __VA_ARGS__)sep()ARPC_FOR_220(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_222(begin, end, what, sep, ...) \
  what(222, __VA_ARGS__)sep()ARPC_FOR_221(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_223(begin, end, what, sep, ...) \
  what(223, __VA_ARGS__)sep()ARPC_FOR_222(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_224(begin, end, what, sep, ...) \
  what(224, __VA_ARGS__)sep()ARPC_FOR_223(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_225(begin, end, what, sep, ...) \
  what(225, __VA_ARGS__)sep()ARPC_FOR_224(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_226(begin, end, what, sep, ...) \
  what(226, __VA_ARGS__)sep()ARPC_FOR_225(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_227(begin, end, what, sep, ...) \
  what(227, __VA_ARGS__)sep()ARPC_FOR_226(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_228(begin, end, what, sep, ...) \
  what(228, __VA_ARGS__)sep()ARPC_FOR_227(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_229(begin, end, what, sep, ...) \
  what(229, __VA_ARGS__)sep()ARPC_FOR_228(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_230(begin, end, what, sep, ...) \
  what(230, __VA_ARGS__)sep()ARPC_FOR_229(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_231(begin, end, what, sep, ...) \
  what(231, __VA_ARGS__)sep()ARPC_FOR_230(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_232(begin, end, what, sep, ...) \
  what(232, __VA_ARGS__)sep()ARPC_FOR_231(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_233(begin, end, what, sep, ...) \
  what(233, __VA_ARGS__)sep()ARPC_FOR_232(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_234(begin, end, what, sep, ...) \
  what(234, __VA_ARGS__)sep()ARPC_FOR_233(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_235(begin, end, what, sep, ...) \
  what(235, __VA_ARGS__)sep()ARPC_FOR_234(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_236(begin, end, what, sep, ...) \
  what(236, __VA_ARGS__)sep()ARPC_FOR_235(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_237(begin, end, what, sep, ...) \
  what(237, __VA_ARGS__)sep()ARPC_FOR_236(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_238(begin, end, what, sep, ...) \
  what(238, __VA_ARGS__)sep()ARPC_FOR_237(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_239(begin, end, what, sep, ...) \
  what(239, __VA_ARGS__)sep()ARPC_FOR_238(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_240(begin, end, what, sep, ...) \
  what(240, __VA_ARGS__)sep()ARPC_FOR_239(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_241(begin, end, what, sep, ...) \
  what(241, __VA_ARGS__)sep()ARPC_FOR_240(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_242(begin, end, what, sep, ...) \
  what(242, __VA_ARGS__)sep()ARPC_FOR_241(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_243(begin, end, what, sep, ...) \
  what(243, __VA_ARGS__)sep()ARPC_FOR_242(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_244(begin, end, what, sep, ...) \
  what(244, __VA_ARGS__)sep()ARPC_FOR_243(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_245(begin, end, what, sep, ...) \
  what(245, __VA_ARGS__)sep()ARPC_FOR_244(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_246(begin, end, what, sep, ...) \
  what(246, __VA_ARGS__)sep()ARPC_FOR_245(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_247(begin, end, what, sep, ...) \
  what(247, __VA_ARGS__)sep()ARPC_FOR_246(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_248(begin, end, what, sep, ...) \
  what(248, __VA_ARGS__)sep()ARPC_FOR_247(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_249(begin, end, what, sep, ...) \
  what(249, __VA_ARGS__)sep()ARPC_FOR_248(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_250(begin, end, what, sep, ...) \
  what(250, __VA_ARGS__)sep()ARPC_FOR_249(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_251(begin, end, what, sep, ...) \
  what(251, __VA_ARGS__)sep()ARPC_FOR_250(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_252(begin, end, what, sep, ...) \
  what(252, __VA_ARGS__)sep()ARPC_FOR_251(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_253(begin, end, what, sep, ...) \
  what(253, __VA_ARGS__)sep()ARPC_FOR_252(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_254(begin, end, what, sep, ...) \
  what(254, __VA_ARGS__)sep()ARPC_FOR_253(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_255(begin, end, what, sep, ...) \
  what(255, __VA_ARGS__)sep()ARPC_FOR_254(begin, end, what, sep, __VA_ARGS__)
#define ARPC_FOR_256(begin, end, what, sep, ...) \
  what(256, __VA_ARGS__)sep()ARPC_FOR_255(begin, end, what, sep, __VA_ARGS__)

// clang-format on

#endif  // ARPC_PREPROCESSOR_H_
