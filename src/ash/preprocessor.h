/// \file
/// \brief Preprocessor-based iteration and code generation.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#ifndef LASR_PREPROCESSOR_H_
#define LASR_PREPROCESSOR_H_

#define LASR_MATCH_ARG_N(                                                       \
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
#define LASR_REV_N_SEQ()                                                       \
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
#define LASR_COMMA_PAD_SEQ()                                                   \
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

#define LASR_HAS_COMMA(...) \
  LASR_HAS_COMMA_HELPER(__VA_ARGS__, LASR_COMMA_PAD_SEQ())
#define LASR_HAS_COMMA_HELPER(...) LASR_MATCH_ARG_N(__VA_ARGS__)

#define LASR_TRIGGER_PARENTHESIS(...) ,
#define LASR_IS_EMPTY(...)                                                 \
  LASR_IS_EMPTY_HELPER(LASR_HAS_COMMA(__VA_ARGS__),                         \
                      LASR_HAS_COMMA(LASR_TRIGGER_PARENTHESIS __VA_ARGS__), \
                      LASR_HAS_COMMA(__VA_ARGS__()),                       \
                      LASR_HAS_COMMA(LASR_TRIGGER_PARENTHESIS __VA_ARGS__()))
#define LASR_IS_EMPTY_CASE_0001 ,
#define LASR_PASTE_5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define LASR_IS_EMPTY_HELPER(_0, _1, _2, _3) \
  LASR_HAS_COMMA(LASR_PASTE_5(LASR_IS_EMPTY_CASE_, _0, _1, _2, _3))

// Concatenate two symbols.
#define LASR_CONCAT(a, b) a##b

// Conditionals.
#define LASR_SECOND(a, b, ...) b
#define LASR_IS_PROBE(...) LASR_SECOND(__VA_ARGS__, 0)
#define LASR_PROBE() ~, 1

#define LASR_NOT(x) LASR_IS_PROBE(LASR_CONCAT(LASR_NOT_HELPER_, x))
#define LASR_NOT_HELPER_0 LASR_PROBE()
#define LASR_BOOL(x) LASR_NOT(LASR_NOT(x))

#define LASR_IF(condition) LASR_IF_HELPER(LASR_BOOL(condition))
#define LASR_IF_HELPER(condition) LASR_CONCAT(LASR_IF_THEN_, condition)
#define LASR_IF_THEN_0(...)
#define LASR_IF_THEN_1(...) __VA_ARGS__

#define LASR_IF_ELSE(condition) LASR_IF_ELSE_HELPER(LASR_BOOL(condition))
#define LASR_IF_ELSE_HELPER(condition) LASR_CONCAT(LASR_IF_THEN_ELSE_, condition)
#define LASR_IF_THEN_ELSE_0(...) LASR_IF_ELSE_0
#define LASR_IF_THEN_ELSE_1(...) __VA_ARGS__ LASR_IF_ELSE_1
#define LASR_IF_ELSE_0(...) __VA_ARGS__
#define LASR_IF_ELSE_1(...)

// Convert to a string.
#define LASR_STRINGIZE(s) #s

// Expand to nothing.
#define LASR_EMPTY()

// Remove parenthesis.
#define LASR_NO_PARENS(MACRO, ...) MACRO __VA_ARGS__

// Defer expansion.
#define LASR_DEFER_1(MACRO) MACRO LASR_EMPTY()
#define LASR_DEFER_2(MACRO) MACRO LASR_EMPTY LASR_EMPTY()()
#define LASR_DEFER_3(MACRO) MACRO LASR_EMPTY LASR_EMPTY LASR_EMPTY()()()
#define LASR_DEFER_4(MACRO) MACRO LASR_EMPTY LASR_EMPTY LASR_EMPTY LASR_EMPTY()()()()

// Force recursive expansion.
#define LASR_EXPAND_EMPTY
#define LASR_EXPAND(...) LASR_EXPAND_1024(__VA_ARGS__)
#define LASR_EXPAND_1024(...) LASR_EXPAND_512(LASR_EXPAND_512(__VA_ARGS__))
#define LASR_EXPAND_512(...) LASR_EXPAND_256(LASR_EXPAND_256(__VA_ARGS__))
#define LASR_EXPAND_256(...) LASR_EXPAND_128(LASR_EXPAND_128(__VA_ARGS__))
#define LASR_EXPAND_128(...) LASR_EXPAND_64(LASR_EXPAND_64(__VA_ARGS__))
#define LASR_EXPAND_64(...) LASR_EXPAND_32(LASR_EXPAND_32(__VA_ARGS__))
#define LASR_EXPAND_32(...) LASR_EXPAND_16(LASR_EXPAND_16(__VA_ARGS__))
#define LASR_EXPAND_16(...) LASR_EXPAND_8(LASR_EXPAND_8(__VA_ARGS__))
#define LASR_EXPAND_8(...) LASR_EXPAND_4(LASR_EXPAND_4(__VA_ARGS__))
#define LASR_EXPAND_4(...) LASR_EXPAND_2(LASR_EXPAND_2(__VA_ARGS__))
#define LASR_EXPAND_2(...) LASR_EXPAND_1(LASR_EXPAND_1(__VA_ARGS__))
#define LASR_EXPAND_1(...) __VA_ARGS__

/// Count the number of arguments (actually commas, as no arguments counts as
/// one) in the variadic args.
#define LASR_ARG_COUNT(...)               \
  LASR_IF_ELSE(LASR_IS_EMPTY(__VA_ARGS__)) \
  (0)(LASR_ARG_COUNT_HELPER(__VA_ARGS__, LASR_REV_N_SEQ()))
#define LASR_ARG_COUNT_HELPER(...) LASR_MATCH_ARG_N(__VA_ARGS__)

// Expand what(x) for every x in the variadic args, with sep() between each two.
#define LASR_FOREACH(what, sep, ...) \
  LASR_FOREACH_N(LASR_ARG_COUNT(__VA_ARGS__), what, sep, __VA_ARGS__)
#define LASR_FOREACH_EXPAND(...) __VA_ARGS__
#define LASR_FOREACH_N(n, what, sep, ...) \
  LASR_FOREACH_EXPAND(LASR_CONCAT(LASR_FOREACH_, n)(what, sep, __VA_ARGS__))

// For deferred invocation:
#define LASR_FOREACH_AGAIN() LASR_FOREACH

// clang-format off
#define LASR_FOREACH_0(what, sep, ...)
#define LASR_FOREACH_1(what, sep, first, ...) what(first)
#define LASR_FOREACH_2(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_1(what, sep, __VA_ARGS__)
#define LASR_FOREACH_3(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_2(what, sep, __VA_ARGS__)
#define LASR_FOREACH_4(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_3(what, sep, __VA_ARGS__)
#define LASR_FOREACH_5(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_4(what, sep, __VA_ARGS__)
#define LASR_FOREACH_6(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_5(what, sep, __VA_ARGS__)
#define LASR_FOREACH_7(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_6(what, sep, __VA_ARGS__)
#define LASR_FOREACH_8(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_7(what, sep, __VA_ARGS__)
#define LASR_FOREACH_9(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_8(what, sep, __VA_ARGS__)
#define LASR_FOREACH_10(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_9(what, sep, __VA_ARGS__)
#define LASR_FOREACH_11(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_10(what, sep, __VA_ARGS__)
#define LASR_FOREACH_12(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_11(what, sep, __VA_ARGS__)
#define LASR_FOREACH_13(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_12(what, sep, __VA_ARGS__)
#define LASR_FOREACH_14(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_13(what, sep, __VA_ARGS__)
#define LASR_FOREACH_15(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_14(what, sep, __VA_ARGS__)
#define LASR_FOREACH_16(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_15(what, sep, __VA_ARGS__)
#define LASR_FOREACH_17(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_16(what, sep, __VA_ARGS__)
#define LASR_FOREACH_18(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_17(what, sep, __VA_ARGS__)
#define LASR_FOREACH_19(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_18(what, sep, __VA_ARGS__)
#define LASR_FOREACH_20(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_19(what, sep, __VA_ARGS__)
#define LASR_FOREACH_21(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_20(what, sep, __VA_ARGS__)
#define LASR_FOREACH_22(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_21(what, sep, __VA_ARGS__)
#define LASR_FOREACH_23(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_22(what, sep, __VA_ARGS__)
#define LASR_FOREACH_24(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_23(what, sep, __VA_ARGS__)
#define LASR_FOREACH_25(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_24(what, sep, __VA_ARGS__)
#define LASR_FOREACH_26(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_25(what, sep, __VA_ARGS__)
#define LASR_FOREACH_27(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_26(what, sep, __VA_ARGS__)
#define LASR_FOREACH_28(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_27(what, sep, __VA_ARGS__)
#define LASR_FOREACH_29(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_28(what, sep, __VA_ARGS__)
#define LASR_FOREACH_30(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_29(what, sep, __VA_ARGS__)
#define LASR_FOREACH_31(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_30(what, sep, __VA_ARGS__)
#define LASR_FOREACH_32(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_31(what, sep, __VA_ARGS__)
#define LASR_FOREACH_33(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_32(what, sep, __VA_ARGS__)
#define LASR_FOREACH_34(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_33(what, sep, __VA_ARGS__)
#define LASR_FOREACH_35(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_34(what, sep, __VA_ARGS__)
#define LASR_FOREACH_36(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_35(what, sep, __VA_ARGS__)
#define LASR_FOREACH_37(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_36(what, sep, __VA_ARGS__)
#define LASR_FOREACH_38(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_37(what, sep, __VA_ARGS__)
#define LASR_FOREACH_39(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_38(what, sep, __VA_ARGS__)
#define LASR_FOREACH_40(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_39(what, sep, __VA_ARGS__)
#define LASR_FOREACH_41(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_40(what, sep, __VA_ARGS__)
#define LASR_FOREACH_42(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_41(what, sep, __VA_ARGS__)
#define LASR_FOREACH_43(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_42(what, sep, __VA_ARGS__)
#define LASR_FOREACH_44(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_43(what, sep, __VA_ARGS__)
#define LASR_FOREACH_45(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_44(what, sep, __VA_ARGS__)
#define LASR_FOREACH_46(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_45(what, sep, __VA_ARGS__)
#define LASR_FOREACH_47(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_46(what, sep, __VA_ARGS__)
#define LASR_FOREACH_48(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_47(what, sep, __VA_ARGS__)
#define LASR_FOREACH_49(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_48(what, sep, __VA_ARGS__)
#define LASR_FOREACH_50(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_49(what, sep, __VA_ARGS__)
#define LASR_FOREACH_51(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_50(what, sep, __VA_ARGS__)
#define LASR_FOREACH_52(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_51(what, sep, __VA_ARGS__)
#define LASR_FOREACH_53(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_52(what, sep, __VA_ARGS__)
#define LASR_FOREACH_54(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_53(what, sep, __VA_ARGS__)
#define LASR_FOREACH_55(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_54(what, sep, __VA_ARGS__)
#define LASR_FOREACH_56(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_55(what, sep, __VA_ARGS__)
#define LASR_FOREACH_57(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_56(what, sep, __VA_ARGS__)
#define LASR_FOREACH_58(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_57(what, sep, __VA_ARGS__)
#define LASR_FOREACH_59(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_58(what, sep, __VA_ARGS__)
#define LASR_FOREACH_60(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_59(what, sep, __VA_ARGS__)
#define LASR_FOREACH_61(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_60(what, sep, __VA_ARGS__)
#define LASR_FOREACH_62(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_61(what, sep, __VA_ARGS__)
#define LASR_FOREACH_63(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_62(what, sep, __VA_ARGS__)
#define LASR_FOREACH_64(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_63(what, sep, __VA_ARGS__)
#define LASR_FOREACH_65(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_64(what, sep, __VA_ARGS__)
#define LASR_FOREACH_66(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_65(what, sep, __VA_ARGS__)
#define LASR_FOREACH_67(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_66(what, sep, __VA_ARGS__)
#define LASR_FOREACH_68(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_67(what, sep, __VA_ARGS__)
#define LASR_FOREACH_69(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_68(what, sep, __VA_ARGS__)
#define LASR_FOREACH_70(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_69(what, sep, __VA_ARGS__)
#define LASR_FOREACH_71(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_70(what, sep, __VA_ARGS__)
#define LASR_FOREACH_72(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_71(what, sep, __VA_ARGS__)
#define LASR_FOREACH_73(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_72(what, sep, __VA_ARGS__)
#define LASR_FOREACH_74(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_73(what, sep, __VA_ARGS__)
#define LASR_FOREACH_75(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_74(what, sep, __VA_ARGS__)
#define LASR_FOREACH_76(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_75(what, sep, __VA_ARGS__)
#define LASR_FOREACH_77(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_76(what, sep, __VA_ARGS__)
#define LASR_FOREACH_78(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_77(what, sep, __VA_ARGS__)
#define LASR_FOREACH_79(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_78(what, sep, __VA_ARGS__)
#define LASR_FOREACH_80(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_79(what, sep, __VA_ARGS__)
#define LASR_FOREACH_81(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_80(what, sep, __VA_ARGS__)
#define LASR_FOREACH_82(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_81(what, sep, __VA_ARGS__)
#define LASR_FOREACH_83(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_82(what, sep, __VA_ARGS__)
#define LASR_FOREACH_84(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_83(what, sep, __VA_ARGS__)
#define LASR_FOREACH_85(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_84(what, sep, __VA_ARGS__)
#define LASR_FOREACH_86(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_85(what, sep, __VA_ARGS__)
#define LASR_FOREACH_87(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_86(what, sep, __VA_ARGS__)
#define LASR_FOREACH_88(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_87(what, sep, __VA_ARGS__)
#define LASR_FOREACH_89(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_88(what, sep, __VA_ARGS__)
#define LASR_FOREACH_90(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_89(what, sep, __VA_ARGS__)
#define LASR_FOREACH_91(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_90(what, sep, __VA_ARGS__)
#define LASR_FOREACH_92(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_91(what, sep, __VA_ARGS__)
#define LASR_FOREACH_93(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_92(what, sep, __VA_ARGS__)
#define LASR_FOREACH_94(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_93(what, sep, __VA_ARGS__)
#define LASR_FOREACH_95(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_94(what, sep, __VA_ARGS__)
#define LASR_FOREACH_96(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_95(what, sep, __VA_ARGS__)
#define LASR_FOREACH_97(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_96(what, sep, __VA_ARGS__)
#define LASR_FOREACH_98(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_97(what, sep, __VA_ARGS__)
#define LASR_FOREACH_99(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_98(what, sep, __VA_ARGS__)
#define LASR_FOREACH_100(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_99(what, sep, __VA_ARGS__)
#define LASR_FOREACH_101(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_100(what, sep, __VA_ARGS__)
#define LASR_FOREACH_102(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_101(what, sep, __VA_ARGS__)
#define LASR_FOREACH_103(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_102(what, sep, __VA_ARGS__)
#define LASR_FOREACH_104(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_103(what, sep, __VA_ARGS__)
#define LASR_FOREACH_105(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_104(what, sep, __VA_ARGS__)
#define LASR_FOREACH_106(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_105(what, sep, __VA_ARGS__)
#define LASR_FOREACH_107(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_106(what, sep, __VA_ARGS__)
#define LASR_FOREACH_108(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_107(what, sep, __VA_ARGS__)
#define LASR_FOREACH_109(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_108(what, sep, __VA_ARGS__)
#define LASR_FOREACH_110(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_109(what, sep, __VA_ARGS__)
#define LASR_FOREACH_111(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_110(what, sep, __VA_ARGS__)
#define LASR_FOREACH_112(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_111(what, sep, __VA_ARGS__)
#define LASR_FOREACH_113(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_112(what, sep, __VA_ARGS__)
#define LASR_FOREACH_114(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_113(what, sep, __VA_ARGS__)
#define LASR_FOREACH_115(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_114(what, sep, __VA_ARGS__)
#define LASR_FOREACH_116(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_115(what, sep, __VA_ARGS__)
#define LASR_FOREACH_117(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_116(what, sep, __VA_ARGS__)
#define LASR_FOREACH_118(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_117(what, sep, __VA_ARGS__)
#define LASR_FOREACH_119(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_118(what, sep, __VA_ARGS__)
#define LASR_FOREACH_120(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_119(what, sep, __VA_ARGS__)
#define LASR_FOREACH_121(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_120(what, sep, __VA_ARGS__)
#define LASR_FOREACH_122(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_121(what, sep, __VA_ARGS__)
#define LASR_FOREACH_123(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_122(what, sep, __VA_ARGS__)
#define LASR_FOREACH_124(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_123(what, sep, __VA_ARGS__)
#define LASR_FOREACH_125(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_124(what, sep, __VA_ARGS__)
#define LASR_FOREACH_126(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_125(what, sep, __VA_ARGS__)
#define LASR_FOREACH_127(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_126(what, sep, __VA_ARGS__)
#define LASR_FOREACH_128(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_127(what, sep, __VA_ARGS__)
#define LASR_FOREACH_129(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_128(what, sep, __VA_ARGS__)
#define LASR_FOREACH_130(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_129(what, sep, __VA_ARGS__)
#define LASR_FOREACH_131(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_130(what, sep, __VA_ARGS__)
#define LASR_FOREACH_132(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_131(what, sep, __VA_ARGS__)
#define LASR_FOREACH_133(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_132(what, sep, __VA_ARGS__)
#define LASR_FOREACH_134(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_133(what, sep, __VA_ARGS__)
#define LASR_FOREACH_135(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_134(what, sep, __VA_ARGS__)
#define LASR_FOREACH_136(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_135(what, sep, __VA_ARGS__)
#define LASR_FOREACH_137(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_136(what, sep, __VA_ARGS__)
#define LASR_FOREACH_138(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_137(what, sep, __VA_ARGS__)
#define LASR_FOREACH_139(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_138(what, sep, __VA_ARGS__)
#define LASR_FOREACH_140(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_139(what, sep, __VA_ARGS__)
#define LASR_FOREACH_141(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_140(what, sep, __VA_ARGS__)
#define LASR_FOREACH_142(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_141(what, sep, __VA_ARGS__)
#define LASR_FOREACH_143(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_142(what, sep, __VA_ARGS__)
#define LASR_FOREACH_144(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_143(what, sep, __VA_ARGS__)
#define LASR_FOREACH_145(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_144(what, sep, __VA_ARGS__)
#define LASR_FOREACH_146(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_145(what, sep, __VA_ARGS__)
#define LASR_FOREACH_147(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_146(what, sep, __VA_ARGS__)
#define LASR_FOREACH_148(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_147(what, sep, __VA_ARGS__)
#define LASR_FOREACH_149(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_148(what, sep, __VA_ARGS__)
#define LASR_FOREACH_150(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_149(what, sep, __VA_ARGS__)
#define LASR_FOREACH_151(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_150(what, sep, __VA_ARGS__)
#define LASR_FOREACH_152(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_151(what, sep, __VA_ARGS__)
#define LASR_FOREACH_153(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_152(what, sep, __VA_ARGS__)
#define LASR_FOREACH_154(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_153(what, sep, __VA_ARGS__)
#define LASR_FOREACH_155(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_154(what, sep, __VA_ARGS__)
#define LASR_FOREACH_156(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_155(what, sep, __VA_ARGS__)
#define LASR_FOREACH_157(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_156(what, sep, __VA_ARGS__)
#define LASR_FOREACH_158(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_157(what, sep, __VA_ARGS__)
#define LASR_FOREACH_159(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_158(what, sep, __VA_ARGS__)
#define LASR_FOREACH_160(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_159(what, sep, __VA_ARGS__)
#define LASR_FOREACH_161(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_160(what, sep, __VA_ARGS__)
#define LASR_FOREACH_162(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_161(what, sep, __VA_ARGS__)
#define LASR_FOREACH_163(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_162(what, sep, __VA_ARGS__)
#define LASR_FOREACH_164(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_163(what, sep, __VA_ARGS__)
#define LASR_FOREACH_165(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_164(what, sep, __VA_ARGS__)
#define LASR_FOREACH_166(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_165(what, sep, __VA_ARGS__)
#define LASR_FOREACH_167(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_166(what, sep, __VA_ARGS__)
#define LASR_FOREACH_168(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_167(what, sep, __VA_ARGS__)
#define LASR_FOREACH_169(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_168(what, sep, __VA_ARGS__)
#define LASR_FOREACH_170(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_169(what, sep, __VA_ARGS__)
#define LASR_FOREACH_171(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_170(what, sep, __VA_ARGS__)
#define LASR_FOREACH_172(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_171(what, sep, __VA_ARGS__)
#define LASR_FOREACH_173(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_172(what, sep, __VA_ARGS__)
#define LASR_FOREACH_174(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_173(what, sep, __VA_ARGS__)
#define LASR_FOREACH_175(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_174(what, sep, __VA_ARGS__)
#define LASR_FOREACH_176(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_175(what, sep, __VA_ARGS__)
#define LASR_FOREACH_177(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_176(what, sep, __VA_ARGS__)
#define LASR_FOREACH_178(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_177(what, sep, __VA_ARGS__)
#define LASR_FOREACH_179(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_178(what, sep, __VA_ARGS__)
#define LASR_FOREACH_180(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_179(what, sep, __VA_ARGS__)
#define LASR_FOREACH_181(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_180(what, sep, __VA_ARGS__)
#define LASR_FOREACH_182(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_181(what, sep, __VA_ARGS__)
#define LASR_FOREACH_183(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_182(what, sep, __VA_ARGS__)
#define LASR_FOREACH_184(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_183(what, sep, __VA_ARGS__)
#define LASR_FOREACH_185(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_184(what, sep, __VA_ARGS__)
#define LASR_FOREACH_186(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_185(what, sep, __VA_ARGS__)
#define LASR_FOREACH_187(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_186(what, sep, __VA_ARGS__)
#define LASR_FOREACH_188(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_187(what, sep, __VA_ARGS__)
#define LASR_FOREACH_189(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_188(what, sep, __VA_ARGS__)
#define LASR_FOREACH_190(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_189(what, sep, __VA_ARGS__)
#define LASR_FOREACH_191(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_190(what, sep, __VA_ARGS__)
#define LASR_FOREACH_192(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_191(what, sep, __VA_ARGS__)
#define LASR_FOREACH_193(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_192(what, sep, __VA_ARGS__)
#define LASR_FOREACH_194(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_193(what, sep, __VA_ARGS__)
#define LASR_FOREACH_195(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_194(what, sep, __VA_ARGS__)
#define LASR_FOREACH_196(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_195(what, sep, __VA_ARGS__)
#define LASR_FOREACH_197(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_196(what, sep, __VA_ARGS__)
#define LASR_FOREACH_198(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_197(what, sep, __VA_ARGS__)
#define LASR_FOREACH_199(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_198(what, sep, __VA_ARGS__)
#define LASR_FOREACH_200(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_199(what, sep, __VA_ARGS__)
#define LASR_FOREACH_201(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_200(what, sep, __VA_ARGS__)
#define LASR_FOREACH_202(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_201(what, sep, __VA_ARGS__)
#define LASR_FOREACH_203(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_202(what, sep, __VA_ARGS__)
#define LASR_FOREACH_204(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_203(what, sep, __VA_ARGS__)
#define LASR_FOREACH_205(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_204(what, sep, __VA_ARGS__)
#define LASR_FOREACH_206(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_205(what, sep, __VA_ARGS__)
#define LASR_FOREACH_207(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_206(what, sep, __VA_ARGS__)
#define LASR_FOREACH_208(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_207(what, sep, __VA_ARGS__)
#define LASR_FOREACH_209(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_208(what, sep, __VA_ARGS__)
#define LASR_FOREACH_210(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_209(what, sep, __VA_ARGS__)
#define LASR_FOREACH_211(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_210(what, sep, __VA_ARGS__)
#define LASR_FOREACH_212(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_211(what, sep, __VA_ARGS__)
#define LASR_FOREACH_213(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_212(what, sep, __VA_ARGS__)
#define LASR_FOREACH_214(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_213(what, sep, __VA_ARGS__)
#define LASR_FOREACH_215(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_214(what, sep, __VA_ARGS__)
#define LASR_FOREACH_216(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_215(what, sep, __VA_ARGS__)
#define LASR_FOREACH_217(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_216(what, sep, __VA_ARGS__)
#define LASR_FOREACH_218(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_217(what, sep, __VA_ARGS__)
#define LASR_FOREACH_219(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_218(what, sep, __VA_ARGS__)
#define LASR_FOREACH_220(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_219(what, sep, __VA_ARGS__)
#define LASR_FOREACH_221(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_220(what, sep, __VA_ARGS__)
#define LASR_FOREACH_222(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_221(what, sep, __VA_ARGS__)
#define LASR_FOREACH_223(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_222(what, sep, __VA_ARGS__)
#define LASR_FOREACH_224(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_223(what, sep, __VA_ARGS__)
#define LASR_FOREACH_225(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_224(what, sep, __VA_ARGS__)
#define LASR_FOREACH_226(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_225(what, sep, __VA_ARGS__)
#define LASR_FOREACH_227(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_226(what, sep, __VA_ARGS__)
#define LASR_FOREACH_228(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_227(what, sep, __VA_ARGS__)
#define LASR_FOREACH_229(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_228(what, sep, __VA_ARGS__)
#define LASR_FOREACH_230(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_229(what, sep, __VA_ARGS__)
#define LASR_FOREACH_231(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_230(what, sep, __VA_ARGS__)
#define LASR_FOREACH_232(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_231(what, sep, __VA_ARGS__)
#define LASR_FOREACH_233(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_232(what, sep, __VA_ARGS__)
#define LASR_FOREACH_234(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_233(what, sep, __VA_ARGS__)
#define LASR_FOREACH_235(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_234(what, sep, __VA_ARGS__)
#define LASR_FOREACH_236(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_235(what, sep, __VA_ARGS__)
#define LASR_FOREACH_237(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_236(what, sep, __VA_ARGS__)
#define LASR_FOREACH_238(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_237(what, sep, __VA_ARGS__)
#define LASR_FOREACH_239(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_238(what, sep, __VA_ARGS__)
#define LASR_FOREACH_240(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_239(what, sep, __VA_ARGS__)
#define LASR_FOREACH_241(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_240(what, sep, __VA_ARGS__)
#define LASR_FOREACH_242(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_241(what, sep, __VA_ARGS__)
#define LASR_FOREACH_243(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_242(what, sep, __VA_ARGS__)
#define LASR_FOREACH_244(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_243(what, sep, __VA_ARGS__)
#define LASR_FOREACH_245(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_244(what, sep, __VA_ARGS__)
#define LASR_FOREACH_246(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_245(what, sep, __VA_ARGS__)
#define LASR_FOREACH_247(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_246(what, sep, __VA_ARGS__)
#define LASR_FOREACH_248(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_247(what, sep, __VA_ARGS__)
#define LASR_FOREACH_249(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_248(what, sep, __VA_ARGS__)
#define LASR_FOREACH_250(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_249(what, sep, __VA_ARGS__)
#define LASR_FOREACH_251(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_250(what, sep, __VA_ARGS__)
#define LASR_FOREACH_252(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_251(what, sep, __VA_ARGS__)
#define LASR_FOREACH_253(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_252(what, sep, __VA_ARGS__)
#define LASR_FOREACH_254(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_253(what, sep, __VA_ARGS__)
#define LASR_FOREACH_255(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_254(what, sep, __VA_ARGS__)
#define LASR_FOREACH_256(what, sep, first, ...) \
  what(first)sep() LASR_FOREACH_255(what, sep, __VA_ARGS__)
// clang-format on

#endif  // LASR_PREPROCESSOR_H_
