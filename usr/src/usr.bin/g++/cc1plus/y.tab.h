#define IDENTIFIER 257
#define TYPENAME 258
#define SCSPEC 259
#define TYPESPEC 260
#define TYPE_QUAL 261
#define CONSTANT 262
#define STRING 263
#define ELLIPSIS 264
#define SIZEOF 265
#define ENUM 266
#define IF 267
#define ELSE 268
#define WHILE 269
#define DO 270
#define FOR 271
#define SWITCH 272
#define CASE 273
#define DEFAULT 274
#define BREAK 275
#define CONTINUE 276
#define RETURN 277
#define GOTO 278
#define ASM 279
#define TYPEOF 280
#define ALIGNOF 281
#define ATTRIBUTE 282
#define AGGR 283
#define DELETE 284
#define NEW 285
#define OVERLOAD 286
#define PRIVATE 287
#define PUBLIC 288
#define PROTECTED 289
#define THIS 290
#define OPERATOR 291
#define DYNAMIC 292
#define POINTSAT_LEFT_RIGHT 293
#define LEFT_RIGHT 294
#define SCOPE 295
#define EMPTY 296
#define TYPENAME_COLON 297
#define ASSIGN 298
#define RANGE 299
#define OROR 300
#define ANDAND 301
#define MIN_MAX 302
#define EQCOMPARE 303
#define ARITHCOMPARE 304
#define LSHIFT 305
#define RSHIFT 306
#define UNARY 307
#define PLUSPLUS 308
#define MINUSMINUS 309
#define HYPERUNARY 310
#define PAREN_STAR_PAREN 311
#define PAREN_X_SCOPE_STAR_PAREN 312
#define PAREN_X_SCOPE_REF_PAREN 313
#define POINTSAT 314
#define RAISE 315
#define RAISES 316
#define RERAISE 317
#define TRY 318
#define EXCEPT 319
#define CATCH 320
#define TYPENAME_SCOPE 321
#define TYPENAME_ELLIPSIS 322
#define PRE_PARSED_FUNCTION_DECL 323
#define EXTERN_LANG_STRING 324
#define ALL 325
typedef union {long itype; tree ttype; enum tree_code code; } YYSTYPE;
extern YYSTYPE yylval;
