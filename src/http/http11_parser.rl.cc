#line 1 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
/**
*
* Copyright (c) 2010, Zed A. Shaw and Mongrel2 Project Contributors.
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
* 
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
* 
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
* 
*     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
*       of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written
*       permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
* IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "http11_parser.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#include <dbg.h>

#define LEN(AT, FPC) (FPC - buffer - parser->AT)
#define MARK(M,FPC) (parser->M = (FPC) - buffer)
#define PTR_TO(F) (buffer + parser->F)

/** Machine **/


#line 257 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"


/** Data **/

#line 54 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
static const int http_parser_start = 1;
static const int http_parser_first_final = 348;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 260 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"


int http_parser_init(http_parser *parser) {
	int cs = 0;

#line 65 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
	{
		cs = (int)http_parser_start;
	}
	
#line 264 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"

	parser->cs = cs;
	parser->body_start = 0;
	parser->content_len = 0;
	parser->mark = 0;
	parser->nread = 0;
	parser->field_len = 0;
	parser->field_start = 0;
	parser->xml_sent = 0;
	parser->json_sent = 0;
	
	return(1);
}


/** exec **/
size_t http_parser_execute(http_parser *parser, const char *buffer, size_t len, size_t off)  
{
	if(len == 0) return 0;
	parser->nread = 0;
	parser->mark = 0;
	parser->field_len = 0;
	parser->field_start = 0;
	
	const char *p, *pe;
	int cs = parser->cs;
	
	assert(off <= len && "offset past end of buffer");
	
	p = buffer+off;
	pe = buffer+len;
	
	assert(pe - p == (int)len - (int)off && "pointers aren't same distance");
	

#line 103 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
{
		int _ck;
		switch ( cs ) {
			case 1:
			goto st_case_1;
			case 0:
			goto st_case_0;
			case 2:
			goto st_case_2;
			case 3:
			goto st_case_3;
			case 4:
			goto st_case_4;
			case 5:
			goto st_case_5;
			case 6:
			goto st_case_6;
			case 7:
			goto st_case_7;
			case 8:
			goto st_case_8;
			case 9:
			goto st_case_9;
			case 10:
			goto st_case_10;
			case 11:
			goto st_case_11;
			case 12:
			goto st_case_12;
			case 13:
			goto st_case_13;
			case 14:
			goto st_case_14;
			case 15:
			goto st_case_15;
			case 16:
			goto st_case_16;
			case 17:
			goto st_case_17;
			case 348:
			goto st_case_348;
			case 18:
			goto st_case_18;
			case 19:
			goto st_case_19;
			case 20:
			goto st_case_20;
			case 21:
			goto st_case_21;
			case 22:
			goto st_case_22;
			case 23:
			goto st_case_23;
			case 24:
			goto st_case_24;
			case 25:
			goto st_case_25;
			case 26:
			goto st_case_26;
			case 27:
			goto st_case_27;
			case 28:
			goto st_case_28;
			case 29:
			goto st_case_29;
			case 30:
			goto st_case_30;
			case 31:
			goto st_case_31;
			case 32:
			goto st_case_32;
			case 33:
			goto st_case_33;
			case 34:
			goto st_case_34;
			case 35:
			goto st_case_35;
			case 36:
			goto st_case_36;
			case 37:
			goto st_case_37;
			case 38:
			goto st_case_38;
			case 39:
			goto st_case_39;
			case 40:
			goto st_case_40;
			case 41:
			goto st_case_41;
			case 42:
			goto st_case_42;
			case 43:
			goto st_case_43;
			case 44:
			goto st_case_44;
			case 45:
			goto st_case_45;
			case 46:
			goto st_case_46;
			case 47:
			goto st_case_47;
			case 48:
			goto st_case_48;
			case 49:
			goto st_case_49;
			case 50:
			goto st_case_50;
			case 51:
			goto st_case_51;
			case 52:
			goto st_case_52;
			case 53:
			goto st_case_53;
			case 54:
			goto st_case_54;
			case 55:
			goto st_case_55;
			case 56:
			goto st_case_56;
			case 57:
			goto st_case_57;
			case 58:
			goto st_case_58;
			case 59:
			goto st_case_59;
			case 60:
			goto st_case_60;
			case 61:
			goto st_case_61;
			case 62:
			goto st_case_62;
			case 63:
			goto st_case_63;
			case 64:
			goto st_case_64;
			case 65:
			goto st_case_65;
			case 66:
			goto st_case_66;
			case 67:
			goto st_case_67;
			case 68:
			goto st_case_68;
			case 69:
			goto st_case_69;
			case 70:
			goto st_case_70;
			case 71:
			goto st_case_71;
			case 72:
			goto st_case_72;
			case 73:
			goto st_case_73;
			case 74:
			goto st_case_74;
			case 75:
			goto st_case_75;
			case 76:
			goto st_case_76;
			case 77:
			goto st_case_77;
			case 78:
			goto st_case_78;
			case 79:
			goto st_case_79;
			case 80:
			goto st_case_80;
			case 81:
			goto st_case_81;
			case 82:
			goto st_case_82;
			case 83:
			goto st_case_83;
			case 84:
			goto st_case_84;
			case 85:
			goto st_case_85;
			case 86:
			goto st_case_86;
			case 87:
			goto st_case_87;
			case 88:
			goto st_case_88;
			case 89:
			goto st_case_89;
			case 90:
			goto st_case_90;
			case 91:
			goto st_case_91;
			case 92:
			goto st_case_92;
			case 93:
			goto st_case_93;
			case 94:
			goto st_case_94;
			case 95:
			goto st_case_95;
			case 96:
			goto st_case_96;
			case 97:
			goto st_case_97;
			case 98:
			goto st_case_98;
			case 99:
			goto st_case_99;
			case 100:
			goto st_case_100;
			case 101:
			goto st_case_101;
			case 102:
			goto st_case_102;
			case 103:
			goto st_case_103;
			case 104:
			goto st_case_104;
			case 105:
			goto st_case_105;
			case 106:
			goto st_case_106;
			case 107:
			goto st_case_107;
			case 108:
			goto st_case_108;
			case 109:
			goto st_case_109;
			case 110:
			goto st_case_110;
			case 111:
			goto st_case_111;
			case 112:
			goto st_case_112;
			case 113:
			goto st_case_113;
			case 114:
			goto st_case_114;
			case 115:
			goto st_case_115;
			case 116:
			goto st_case_116;
			case 117:
			goto st_case_117;
			case 118:
			goto st_case_118;
			case 119:
			goto st_case_119;
			case 120:
			goto st_case_120;
			case 121:
			goto st_case_121;
			case 122:
			goto st_case_122;
			case 123:
			goto st_case_123;
			case 124:
			goto st_case_124;
			case 125:
			goto st_case_125;
			case 126:
			goto st_case_126;
			case 127:
			goto st_case_127;
			case 128:
			goto st_case_128;
			case 129:
			goto st_case_129;
			case 130:
			goto st_case_130;
			case 131:
			goto st_case_131;
			case 132:
			goto st_case_132;
			case 133:
			goto st_case_133;
			case 134:
			goto st_case_134;
			case 135:
			goto st_case_135;
			case 136:
			goto st_case_136;
			case 137:
			goto st_case_137;
			case 138:
			goto st_case_138;
			case 139:
			goto st_case_139;
			case 140:
			goto st_case_140;
			case 141:
			goto st_case_141;
			case 142:
			goto st_case_142;
			case 143:
			goto st_case_143;
			case 144:
			goto st_case_144;
			case 145:
			goto st_case_145;
			case 146:
			goto st_case_146;
			case 147:
			goto st_case_147;
			case 148:
			goto st_case_148;
			case 149:
			goto st_case_149;
			case 150:
			goto st_case_150;
			case 151:
			goto st_case_151;
			case 152:
			goto st_case_152;
			case 153:
			goto st_case_153;
			case 154:
			goto st_case_154;
			case 155:
			goto st_case_155;
			case 156:
			goto st_case_156;
			case 157:
			goto st_case_157;
			case 158:
			goto st_case_158;
			case 159:
			goto st_case_159;
			case 160:
			goto st_case_160;
			case 161:
			goto st_case_161;
			case 162:
			goto st_case_162;
			case 163:
			goto st_case_163;
			case 164:
			goto st_case_164;
			case 165:
			goto st_case_165;
			case 166:
			goto st_case_166;
			case 167:
			goto st_case_167;
			case 168:
			goto st_case_168;
			case 169:
			goto st_case_169;
			case 170:
			goto st_case_170;
			case 171:
			goto st_case_171;
			case 172:
			goto st_case_172;
			case 173:
			goto st_case_173;
			case 174:
			goto st_case_174;
			case 175:
			goto st_case_175;
			case 176:
			goto st_case_176;
			case 177:
			goto st_case_177;
			case 178:
			goto st_case_178;
			case 179:
			goto st_case_179;
			case 180:
			goto st_case_180;
			case 181:
			goto st_case_181;
			case 182:
			goto st_case_182;
			case 183:
			goto st_case_183;
			case 184:
			goto st_case_184;
			case 185:
			goto st_case_185;
			case 186:
			goto st_case_186;
			case 187:
			goto st_case_187;
			case 188:
			goto st_case_188;
			case 189:
			goto st_case_189;
			case 190:
			goto st_case_190;
			case 191:
			goto st_case_191;
			case 192:
			goto st_case_192;
			case 193:
			goto st_case_193;
			case 194:
			goto st_case_194;
			case 195:
			goto st_case_195;
			case 196:
			goto st_case_196;
			case 197:
			goto st_case_197;
			case 349:
			goto st_case_349;
			case 198:
			goto st_case_198;
			case 199:
			goto st_case_199;
			case 200:
			goto st_case_200;
			case 201:
			goto st_case_201;
			case 202:
			goto st_case_202;
			case 203:
			goto st_case_203;
			case 204:
			goto st_case_204;
			case 205:
			goto st_case_205;
			case 206:
			goto st_case_206;
			case 207:
			goto st_case_207;
			case 208:
			goto st_case_208;
			case 209:
			goto st_case_209;
			case 210:
			goto st_case_210;
			case 211:
			goto st_case_211;
			case 212:
			goto st_case_212;
			case 213:
			goto st_case_213;
			case 214:
			goto st_case_214;
			case 215:
			goto st_case_215;
			case 216:
			goto st_case_216;
			case 217:
			goto st_case_217;
			case 218:
			goto st_case_218;
			case 219:
			goto st_case_219;
			case 220:
			goto st_case_220;
			case 221:
			goto st_case_221;
			case 222:
			goto st_case_222;
			case 223:
			goto st_case_223;
			case 224:
			goto st_case_224;
			case 225:
			goto st_case_225;
			case 226:
			goto st_case_226;
			case 227:
			goto st_case_227;
			case 228:
			goto st_case_228;
			case 229:
			goto st_case_229;
			case 230:
			goto st_case_230;
			case 231:
			goto st_case_231;
			case 232:
			goto st_case_232;
			case 233:
			goto st_case_233;
			case 234:
			goto st_case_234;
			case 235:
			goto st_case_235;
			case 236:
			goto st_case_236;
			case 237:
			goto st_case_237;
			case 238:
			goto st_case_238;
			case 239:
			goto st_case_239;
			case 240:
			goto st_case_240;
			case 241:
			goto st_case_241;
			case 242:
			goto st_case_242;
			case 243:
			goto st_case_243;
			case 244:
			goto st_case_244;
			case 245:
			goto st_case_245;
			case 246:
			goto st_case_246;
			case 247:
			goto st_case_247;
			case 248:
			goto st_case_248;
			case 249:
			goto st_case_249;
			case 250:
			goto st_case_250;
			case 251:
			goto st_case_251;
			case 252:
			goto st_case_252;
			case 253:
			goto st_case_253;
			case 254:
			goto st_case_254;
			case 255:
			goto st_case_255;
			case 256:
			goto st_case_256;
			case 257:
			goto st_case_257;
			case 258:
			goto st_case_258;
			case 259:
			goto st_case_259;
			case 260:
			goto st_case_260;
			case 261:
			goto st_case_261;
			case 262:
			goto st_case_262;
			case 263:
			goto st_case_263;
			case 264:
			goto st_case_264;
			case 265:
			goto st_case_265;
			case 266:
			goto st_case_266;
			case 267:
			goto st_case_267;
			case 268:
			goto st_case_268;
			case 269:
			goto st_case_269;
			case 270:
			goto st_case_270;
			case 271:
			goto st_case_271;
			case 272:
			goto st_case_272;
			case 273:
			goto st_case_273;
			case 274:
			goto st_case_274;
			case 275:
			goto st_case_275;
			case 276:
			goto st_case_276;
			case 277:
			goto st_case_277;
			case 278:
			goto st_case_278;
			case 279:
			goto st_case_279;
			case 280:
			goto st_case_280;
			case 281:
			goto st_case_281;
			case 282:
			goto st_case_282;
			case 283:
			goto st_case_283;
			case 284:
			goto st_case_284;
			case 285:
			goto st_case_285;
			case 286:
			goto st_case_286;
			case 287:
			goto st_case_287;
			case 288:
			goto st_case_288;
			case 289:
			goto st_case_289;
			case 290:
			goto st_case_290;
			case 291:
			goto st_case_291;
			case 292:
			goto st_case_292;
			case 293:
			goto st_case_293;
			case 294:
			goto st_case_294;
			case 295:
			goto st_case_295;
			case 296:
			goto st_case_296;
			case 297:
			goto st_case_297;
			case 298:
			goto st_case_298;
			case 299:
			goto st_case_299;
			case 300:
			goto st_case_300;
			case 301:
			goto st_case_301;
			case 302:
			goto st_case_302;
			case 303:
			goto st_case_303;
			case 304:
			goto st_case_304;
			case 305:
			goto st_case_305;
			case 306:
			goto st_case_306;
			case 307:
			goto st_case_307;
			case 308:
			goto st_case_308;
			case 309:
			goto st_case_309;
			case 310:
			goto st_case_310;
			case 311:
			goto st_case_311;
			case 312:
			goto st_case_312;
			case 313:
			goto st_case_313;
			case 314:
			goto st_case_314;
			case 315:
			goto st_case_315;
			case 316:
			goto st_case_316;
			case 317:
			goto st_case_317;
			case 318:
			goto st_case_318;
			case 319:
			goto st_case_319;
			case 320:
			goto st_case_320;
			case 321:
			goto st_case_321;
			case 322:
			goto st_case_322;
			case 323:
			goto st_case_323;
			case 324:
			goto st_case_324;
			case 325:
			goto st_case_325;
			case 326:
			goto st_case_326;
			case 327:
			goto st_case_327;
			case 328:
			goto st_case_328;
			case 329:
			goto st_case_329;
			case 330:
			goto st_case_330;
			case 331:
			goto st_case_331;
			case 332:
			goto st_case_332;
			case 333:
			goto st_case_333;
			case 334:
			goto st_case_334;
			case 335:
			goto st_case_335;
			case 336:
			goto st_case_336;
			case 337:
			goto st_case_337;
			case 338:
			goto st_case_338;
			case 339:
			goto st_case_339;
			case 340:
			goto st_case_340;
			case 341:
			goto st_case_341;
			case 342:
			goto st_case_342;
			case 343:
			goto st_case_343;
			case 344:
			goto st_case_344;
			case 345:
			goto st_case_345;
			case 346:
			goto st_case_346;
			case 347:
			goto st_case_347;
		}
		p+= 1;
		st_case_1:
		if ( p == pe )
			goto _out1;
		switch( ( (*( p))) ) {
			case 60: {
				goto _ctr3;
			}
			case 64: {
				goto _ctr4;
			}
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _ctr2;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _ctr2;
		}
		goto _st0;
		_st0:
		st_case_0:
		goto _out0;
		_ctr2:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 835 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st2;
		_st2:
		p+= 1;
		st_case_2:
		if ( p == pe )
			goto _out2;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st175;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st175;
		}
		goto _st0;
		_ctr6:
			{
#line 69 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_method != NULL) 
			parser->request_method(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 861 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st3;
		_st3:
		p+= 1;
		st_case_3:
		if ( p == pe )
			goto _out3;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr9;
			}
			case 33: {
				goto _ctr10;
			}
			case 35: {
				goto _ctr11;
			}
			case 37: {
				goto _ctr12;
			}
			case 47: {
				goto _ctr13;
			}
			case 59: {
				goto _ctr10;
			}
			case 61: {
				goto _ctr10;
			}
			case 63: {
				goto _ctr14;
			}
			case 64: {
				goto _ctr10;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 899 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr16;
				}
				goto _st0;
				
			}
			case 95: {
				goto _ctr10;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 913 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr16;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 924 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr16;
				}
				goto _st0;
				
			}
			case 126: {
				goto _ctr10;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _ctr10;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _ctr15;
			}
		} else {
			goto _ctr15;
		}
		goto _st0;
		_ctr9:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 953 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 961 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 969 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_ctr48:
			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 979 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 987 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_ctr56:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 994 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 79 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->fragment != NULL)
			parser->fragment(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1002 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_ctr60:
			{
#line 79 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->fragment != NULL)
			parser->fragment(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1012 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_ctr70:
			{
#line 84 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(query_start, p); }
		
#line 1019 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 85 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->query_string != NULL)
			parser->query_string(parser->data, PTR_TO(query_start), LEN(query_start, p));
		}
		
#line 1027 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1035 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_ctr75:
			{
#line 85 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->query_string != NULL)
			parser->query_string(parser->data, PTR_TO(query_start), LEN(query_start, p));
		}
		
#line 1045 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1053 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st4;
		_st4:
		p+= 1;
		st_case_4:
		if ( p == pe )
			goto _out4;
		if ( ( (*( p))) == 72 ) {
			goto _ctr19;
		}
		goto _st0;
		_ctr19:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1069 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st5;
		_st5:
		p+= 1;
		st_case_5:
		if ( p == pe )
			goto _out5;
		if ( ( (*( p))) == 84 ) {
			goto _st6;
		}
		goto _st0;
		_st6:
		p+= 1;
		st_case_6:
		if ( p == pe )
			goto _out6;
		if ( ( (*( p))) == 84 ) {
			goto _st7;
		}
		goto _st0;
		_st7:
		p+= 1;
		st_case_7:
		if ( p == pe )
			goto _out7;
		if ( ( (*( p))) == 80 ) {
			goto _st8;
		}
		goto _st0;
		_st8:
		p+= 1;
		st_case_8:
		if ( p == pe )
			goto _out8;
		if ( ( (*( p))) == 47 ) {
			goto _st9;
		}
		goto _st0;
		_st9:
		p+= 1;
		st_case_9:
		if ( p == pe )
			goto _out9;
		if ( ( (*( p))) == 49 ) {
			goto _st10;
		}
		goto _st0;
		_st10:
		p+= 1;
		st_case_10:
		if ( p == pe )
			goto _out10;
		if ( ( (*( p))) == 46 ) {
			goto _st11;
		}
		goto _st0;
		_st11:
		p+= 1;
		st_case_11:
		if ( p == pe )
			goto _out11;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 49 ) {
			goto _st12;
		}
		goto _st0;
		_st12:
		p+= 1;
		st_case_12:
		if ( p == pe )
			goto _out12;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr28;
			}
			case 13: {
				goto _ctr29;
			}
		}
		goto _st0;
		_ctr28:
			{
#line 90 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_version != NULL)
			parser->http_version(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1156 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st13;
		_ctr39:
			{
#line 61 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1163 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 63 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_field != NULL) {
				parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), LEN(mark, p));
			}
		}
		
#line 1172 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st13;
		_ctr42:
			{
#line 63 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_field != NULL) {
				parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), LEN(mark, p));
			}
		}
		
#line 1183 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st13;
		_st13:
		p+= 1;
		st_case_13:
		if ( p == pe )
			goto _out13;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr32;
			}
			case 13: {
				goto _ctr33;
			}
			case 33: {
				goto _ctr31;
			}
			case 124: {
				goto _ctr31;
			}
			case 126: {
				goto _ctr31;
			}
		}
		if ( ( (*( p))) < 42 ) {
			if ( ( (*( p))) < 11 ) {
				if ( 1 <= ( (*( p))) && ( (*( p))) <= 8 ) {
					goto _ctr31;
				}
			} else if ( ( (*( p))) > 31 ) {
				if ( 35 <= ( (*( p))) && ( (*( p))) <= 39 ) {
					goto _ctr31;
				}
			} else {
				goto _ctr31;
			}
		} else if ( ( (*( p))) > 43 ) {
			if ( ( (*( p))) < 48 ) {
				if ( 45 <= ( (*( p))) && ( (*( p))) <= 46 ) {
					goto _ctr31;
				}
			} else if ( ( (*( p))) > 57 ) {
				if ( ( (*( p))) > 90 ) {
					if ( 94 <= ( (*( p))) && ( (*( p))) <= 122 ) {
						goto _ctr31;
					}
				} else if ( ( (*( p))) >= 65 ) {
					goto _ctr31;
				}
			} else {
				goto _ctr31;
			}
		} else {
			goto _ctr31;
		}
		goto _st0;
		_ctr31:
			{
#line 56 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(field_start, p); }
		
#line 1244 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st14;
		_st14:
		p+= 1;
		st_case_14:
		if ( p == pe )
			goto _out14;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st14;
			}
			case 58: {
				goto _ctr35;
			}
			case 124: {
				goto _st14;
			}
			case 126: {
				goto _st14;
			}
		}
		if ( ( (*( p))) < 42 ) {
			if ( ( (*( p))) < 10 ) {
				if ( 1 <= ( (*( p))) && ( (*( p))) <= 8 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 31 ) {
				if ( 35 <= ( (*( p))) && ( (*( p))) <= 39 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else if ( ( (*( p))) > 43 ) {
			if ( ( (*( p))) < 48 ) {
				if ( 45 <= ( (*( p))) && ( (*( p))) <= 46 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 57 ) {
				if ( ( (*( p))) > 90 ) {
					if ( 94 <= ( (*( p))) && ( (*( p))) <= 122 ) {
						goto _st14;
					}
				} else if ( ( (*( p))) >= 65 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else {
			goto _st14;
		}
		goto _st0;
		_ctr35:
			{
#line 57 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			parser->field_len = LEN(field_start, p);
		}
		
#line 1304 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st15;
		_ctr38:
			{
#line 61 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1311 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st15;
		_st15:
		p+= 1;
		st_case_15:
		if ( p == pe )
			goto _out15;
		switch( ( (*( p))) ) {
			case 0: {
				goto _st0;
			}
			case 9: {
				goto _ctr38;
			}
			case 10: {
				goto _ctr39;
			}
			case 13: {
				goto _ctr40;
			}
			case 32: {
				goto _ctr38;
			}
			case 127: {
				goto _st0;
			}
		}
		goto _ctr37;
		_ctr37:
			{
#line 61 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1344 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st16;
		_st16:
		p+= 1;
		st_case_16:
		if ( p == pe )
			goto _out16;
		switch( ( (*( p))) ) {
			case 0: {
				goto _st0;
			}
			case 10: {
				goto _ctr42;
			}
			case 13: {
				goto _ctr43;
			}
			case 127: {
				goto _st0;
			}
		}
		goto _st16;
		_ctr29:
			{
#line 90 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_version != NULL)
			parser->http_version(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1374 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st17;
		_ctr40:
			{
#line 61 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1381 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 63 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_field != NULL) {
				parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), LEN(mark, p));
			}
		}
		
#line 1390 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st17;
		_ctr43:
			{
#line 63 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->http_field != NULL) {
				parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), LEN(mark, p));
			}
		}
		
#line 1401 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st17;
		_st17:
		p+= 1;
		st_case_17:
		if ( p == pe )
			goto _out17;
		if ( ( (*( p))) == 10 ) {
			goto _st13;
		}
		goto _st0;
		_ctr32:
			{
#line 56 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(field_start, p); }
		
#line 1417 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 100 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->xml_sent || parser->json_sent) {
				parser->body_start = PTR_TO(mark) - buffer;
				// +1 includes the \0
				parser->content_len = p - buffer - parser->body_start + 1;
			} else {
				parser->body_start = p - buffer + 1;
				
				if(parser->header_done != NULL) {
					parser->header_done(parser->data, p + 1, pe - p - 1);
				}
			}
			{p+= 1; cs = 348; goto _out;}
		}
		
#line 1435 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st348;
		_ctr46:
			{
#line 100 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->xml_sent || parser->json_sent) {
				parser->body_start = PTR_TO(mark) - buffer;
				// +1 includes the \0
				parser->content_len = p - buffer - parser->body_start + 1;
			} else {
				parser->body_start = p - buffer + 1;
				
				if(parser->header_done != NULL) {
					parser->header_done(parser->data, p + 1, pe - p - 1);
				}
			}
			{p+= 1; cs = 348; goto _out;}
		}
		
#line 1455 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st348;
		_st348:
		p+= 1;
		st_case_348:
		if ( p == pe )
			goto _out348;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st14;
			}
			case 58: {
				goto _ctr35;
			}
			case 124: {
				goto _st14;
			}
			case 126: {
				goto _st14;
			}
		}
		if ( ( (*( p))) < 42 ) {
			if ( ( (*( p))) < 10 ) {
				if ( 1 <= ( (*( p))) && ( (*( p))) <= 8 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 31 ) {
				if ( 35 <= ( (*( p))) && ( (*( p))) <= 39 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else if ( ( (*( p))) > 43 ) {
			if ( ( (*( p))) < 48 ) {
				if ( 45 <= ( (*( p))) && ( (*( p))) <= 46 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 57 ) {
				if ( ( (*( p))) > 90 ) {
					if ( 94 <= ( (*( p))) && ( (*( p))) <= 122 ) {
						goto _st14;
					}
				} else if ( ( (*( p))) >= 65 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else {
			goto _st14;
		}
		goto _st0;
		_ctr33:
			{
#line 56 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(field_start, p); }
		
#line 1513 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st18;
		_st18:
		p+= 1;
		st_case_18:
		if ( p == pe )
			goto _out18;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr46;
			}
			case 33: {
				goto _st14;
			}
			case 58: {
				goto _ctr35;
			}
			case 124: {
				goto _st14;
			}
			case 126: {
				goto _st14;
			}
		}
		if ( ( (*( p))) < 42 ) {
			if ( ( (*( p))) < 11 ) {
				if ( 1 <= ( (*( p))) && ( (*( p))) <= 8 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 31 ) {
				if ( 35 <= ( (*( p))) && ( (*( p))) <= 39 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else if ( ( (*( p))) > 43 ) {
			if ( ( (*( p))) < 48 ) {
				if ( 45 <= ( (*( p))) && ( (*( p))) <= 46 ) {
					goto _st14;
				}
			} else if ( ( (*( p))) > 57 ) {
				if ( ( (*( p))) > 90 ) {
					if ( 94 <= ( (*( p))) && ( (*( p))) <= 122 ) {
						goto _st14;
					}
				} else if ( ( (*( p))) >= 65 ) {
					goto _st14;
				}
			} else {
				goto _st14;
			}
		} else {
			goto _st14;
		}
		goto _st0;
		_ctr16:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1574 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st19;
		_ctr10:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1581 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st19;
		_st19:
		p+= 1;
		st_case_19:
		if ( p == pe )
			goto _out19;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st19;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st24;
			}
			case 47: {
				goto _st26;
			}
			case 59: {
				goto _st19;
			}
			case 61: {
				goto _st19;
			}
			case 63: {
				goto _ctr52;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 1616 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st19;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 1630 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 1641 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st19;
			}
		}
		if ( ( (*( p))) < 64 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st19;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st19;
			}
		} else {
			goto _st19;
		}
		goto _st0;
		_ctr11:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1670 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 1678 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1686 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st20;
		_ctr49:
			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 1696 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1704 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st20;
		_ctr71:
			{
#line 84 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(query_start, p); }
		
#line 1711 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 85 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->query_string != NULL)
			parser->query_string(parser->data, PTR_TO(query_start), LEN(query_start, p));
		}
		
#line 1719 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1727 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st20;
		_ctr76:
			{
#line 85 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->query_string != NULL)
			parser->query_string(parser->data, PTR_TO(query_start), LEN(query_start, p));
		}
		
#line 1737 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 74 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_uri != NULL)
			parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
		}
		
#line 1745 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st20;
		_st20:
		p+= 1;
		st_case_20:
		if ( p == pe )
			goto _out20;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr56;
			}
			case 37: {
				goto _ctr57;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 1768 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr58;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 34 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 35 ) {
			if ( ( (*( p))) > 93 ) {
				if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
					_ck = 0;
					if ( ( parser->uri_relaxed )
#line 1795 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
					_ck += 1;
					if ( 1 <= _ck )
						{goto _ctr58;
					}
					goto _st0;
					
				}
			} else if ( ( (*( p))) >= 91 ) {
				goto _st0;
			}
		} else {
			goto _st0;
		}
		goto _ctr55;
		_ctr58:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1815 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st21;
		_ctr55:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1822 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st21;
		_st21:
		p+= 1;
		st_case_21:
		if ( p == pe )
			goto _out21;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr60;
			}
			case 37: {
				goto _st22;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 1845 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st21;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 34 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 35 ) {
			if ( ( (*( p))) > 93 ) {
				if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
					_ck = 0;
					if ( ( parser->uri_relaxed )
#line 1872 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
					_ck += 1;
					if ( 1 <= _ck )
						{goto _st21;
					}
					goto _st0;
					
				}
			} else if ( ( (*( p))) >= 91 ) {
				goto _st0;
			}
		} else {
			goto _st0;
		}
		goto _st21;
		_ctr57:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1892 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st22;
		_st22:
		p+= 1;
		st_case_22:
		if ( p == pe )
			goto _out22;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st23;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st23;
			}
		} else {
			goto _st23;
		}
		goto _st0;
		_st23:
		p+= 1;
		st_case_23:
		if ( p == pe )
			goto _out23;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st21;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st21;
			}
		} else {
			goto _st21;
		}
		goto _st0;
		_ctr12:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1933 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st24;
		_st24:
		p+= 1;
		st_case_24:
		if ( p == pe )
			goto _out24;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st25;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st25;
			}
		} else {
			goto _st25;
		}
		goto _st0;
		_st25:
		p+= 1;
		st_case_25:
		if ( p == pe )
			goto _out25;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st19;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st19;
			}
		} else {
			goto _st19;
		}
		goto _st0;
		_ctr228:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1974 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st26;
		_ctr226:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 1981 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st26;
		_st26:
		p+= 1;
		st_case_26:
		if ( p == pe )
			goto _out26;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 34: {
				goto _st0;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st27;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 63: {
				goto _ctr52;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2013 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st26;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 91 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 93 ) {
			if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2039 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st26;
				}
				goto _st0;
				
			}
		} else {
			goto _st0;
		}
		goto _st26;
		_ctr227:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 2056 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st27;
		_st27:
		p+= 1;
		st_case_27:
		if ( p == pe )
			goto _out27;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st28;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st28;
			}
		} else {
			goto _st28;
		}
		goto _st0;
		_st28:
		p+= 1;
		st_case_28:
		if ( p == pe )
			goto _out28;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st26;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st26;
			}
		} else {
			goto _st26;
		}
		goto _st0;
		_ctr14:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 2097 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 2105 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st29;
		_ctr52:
			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 2115 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st29;
		_st29:
		p+= 1;
		st_case_29:
		if ( p == pe )
			goto _out29;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr70;
			}
			case 34: {
				goto _st0;
			}
			case 35: {
				goto _ctr71;
			}
			case 37: {
				goto _ctr72;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2144 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr73;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 91 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 93 ) {
			if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2170 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr73;
				}
				goto _st0;
				
			}
		} else {
			goto _st0;
		}
		goto _ctr69;
		_ctr73:
			{
#line 84 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(query_start, p); }
		
#line 2187 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st30;
		_ctr69:
			{
#line 84 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(query_start, p); }
		
#line 2194 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st30;
		_st30:
		p+= 1;
		st_case_30:
		if ( p == pe )
			goto _out30;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr75;
			}
			case 34: {
				goto _st0;
			}
			case 35: {
				goto _ctr76;
			}
			case 37: {
				goto _st31;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2223 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st30;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 91 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 93 ) {
			if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2249 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st30;
				}
				goto _st0;
				
			}
		} else {
			goto _st0;
		}
		goto _st30;
		_ctr72:
			{
#line 84 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(query_start, p); }
		
#line 2266 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st31;
		_st31:
		p+= 1;
		st_case_31:
		if ( p == pe )
			goto _out31;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st32;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st32;
			}
		} else {
			goto _st32;
		}
		goto _st0;
		_st32:
		p+= 1;
		st_case_32:
		if ( p == pe )
			goto _out32;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st30;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st30;
			}
		} else {
			goto _st30;
		}
		goto _st0;
		_ctr13:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 2307 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st33;
		_st33:
		p+= 1;
		st_case_33:
		if ( p == pe )
			goto _out33;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 34: {
				goto _st0;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st27;
			}
			case 47: {
				goto _st34;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 63: {
				goto _ctr52;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2342 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st26;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 91 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 93 ) {
			if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2368 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st26;
				}
				goto _st0;
				
			}
		} else {
			goto _st0;
		}
		goto _st26;
		_st34:
		p+= 1;
		st_case_34:
		if ( p == pe )
			goto _out34;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st35;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st36;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st38;
			}
			case 61: {
				goto _st35;
			}
			case 63: {
				goto _ctr52;
			}
			case 64: {
				goto _st42;
			}
			case 91: {
				goto _st47;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2419 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st35;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2433 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2444 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st35;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st35;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st35;
			}
		} else {
			goto _st35;
		}
		goto _st0;
		_st35:
		p+= 1;
		st_case_35:
		if ( p == pe )
			goto _out35;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st35;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st36;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st38;
			}
			case 61: {
				goto _st35;
			}
			case 63: {
				goto _ctr52;
			}
			case 64: {
				goto _st42;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2504 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st35;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2518 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2529 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st35;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st35;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st35;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st35;
			}
		} else {
			goto _st35;
		}
		goto _st0;
		_st36:
		p+= 1;
		st_case_36:
		if ( p == pe )
			goto _out36;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st37;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st37;
			}
		} else {
			goto _st37;
		}
		goto _st0;
		_st37:
		p+= 1;
		st_case_37:
		if ( p == pe )
			goto _out37;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st35;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st35;
			}
		} else {
			goto _st35;
		}
		goto _st0;
		_st38:
		p+= 1;
		st_case_38:
		if ( p == pe )
			goto _out38;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st39;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st40;
			}
			case 47: {
				goto _st26;
			}
			case 61: {
				goto _st39;
			}
			case 63: {
				goto _ctr52;
			}
			case 64: {
				goto _st42;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2620 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st39;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2634 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2645 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st39;
			}
		}
		if ( ( (*( p))) < 58 ) {
			if ( ( (*( p))) > 46 ) {
				if ( 48 <= ( (*( p))) ) {
					goto _st38;
				}
			} else if ( ( (*( p))) >= 36 ) {
				goto _st39;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st39;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st39;
			}
		} else {
			goto _st39;
		}
		goto _st0;
		_st39:
		p+= 1;
		st_case_39:
		if ( p == pe )
			goto _out39;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st39;
			}
			case 37: {
				goto _st40;
			}
			case 61: {
				goto _st39;
			}
			case 64: {
				goto _st42;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2698 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st39;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2712 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2723 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st39;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st39;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st39;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st39;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st39;
			}
		} else {
			goto _st39;
		}
		goto _st0;
		_st40:
		p+= 1;
		st_case_40:
		if ( p == pe )
			goto _out40;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st41;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st41;
			}
		} else {
			goto _st41;
		}
		goto _st0;
		_st41:
		p+= 1;
		st_case_41:
		if ( p == pe )
			goto _out41;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st39;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st39;
			}
		} else {
			goto _st39;
		}
		goto _st0;
		_st42:
		p+= 1;
		st_case_42:
		if ( p == pe )
			goto _out42;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st43;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st44;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st46;
			}
			case 61: {
				goto _st43;
			}
			case 63: {
				goto _ctr52;
			}
			case 91: {
				goto _st47;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2821 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st43;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2835 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2846 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st43;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st43;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st43;
			}
		} else {
			goto _st43;
		}
		goto _st0;
		_st43:
		p+= 1;
		st_case_43:
		if ( p == pe )
			goto _out43;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st43;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st44;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st46;
			}
			case 61: {
				goto _st43;
			}
			case 63: {
				goto _ctr52;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2903 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st43;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2917 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 2928 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st43;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st43;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st43;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st43;
			}
		} else {
			goto _st43;
		}
		goto _st0;
		_st44:
		p+= 1;
		st_case_44:
		if ( p == pe )
			goto _out44;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st45;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st45;
			}
		} else {
			goto _st45;
		}
		goto _st0;
		_st45:
		p+= 1;
		st_case_45:
		if ( p == pe )
			goto _out45;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st43;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st43;
			}
		} else {
			goto _st43;
		}
		goto _st0;
		_st46:
		p+= 1;
		st_case_46:
		if ( p == pe )
			goto _out46;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 35: {
				goto _ctr49;
			}
			case 47: {
				goto _st26;
			}
			case 63: {
				goto _ctr52;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st46;
		}
		goto _st0;
		_st47:
		p+= 1;
		st_case_47:
		if ( p == pe )
			goto _out47;
		switch( ( (*( p))) ) {
			case 6: {
				goto _st48;
			}
			case 58: {
				goto _st167;
			}
			case 118: {
				goto _st169;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st91;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st91;
			}
		} else {
			goto _st91;
		}
		goto _st0;
		_st48:
		p+= 1;
		st_case_48:
		if ( p == pe )
			goto _out48;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st49;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st49;
			}
		} else {
			goto _st49;
		}
		goto _st0;
		_st49:
		p+= 1;
		st_case_49:
		if ( p == pe )
			goto _out49;
		if ( ( (*( p))) == 58 ) {
			goto _st53;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st50;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st50;
			}
		} else {
			goto _st50;
		}
		goto _st0;
		_st50:
		p+= 1;
		st_case_50:
		if ( p == pe )
			goto _out50;
		if ( ( (*( p))) == 58 ) {
			goto _st53;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st51;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st51;
			}
		} else {
			goto _st51;
		}
		goto _st0;
		_st51:
		p+= 1;
		st_case_51:
		if ( p == pe )
			goto _out51;
		if ( ( (*( p))) == 58 ) {
			goto _st53;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st52;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st52;
			}
		} else {
			goto _st52;
		}
		goto _st0;
		_st52:
		p+= 1;
		st_case_52:
		if ( p == pe )
			goto _out52;
		if ( ( (*( p))) == 58 ) {
			goto _st53;
		}
		goto _st0;
		_st53:
		p+= 1;
		st_case_53:
		if ( p == pe )
			goto _out53;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st84;
			}
			case 50: {
				goto _st87;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st54;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st90;
			}
		} else {
			goto _st90;
		}
		goto _st0;
		_st54:
		p+= 1;
		st_case_54:
		if ( p == pe )
			goto _out54;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st77;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st77;
			}
		} else {
			goto _st77;
		}
		goto _st0;
		_st55:
		p+= 1;
		st_case_55:
		if ( p == pe )
			goto _out55;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st72;
			}
			case 50: {
				goto _st74;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st56;
		}
		goto _st0;
		_st56:
		p+= 1;
		st_case_56:
		if ( p == pe )
			goto _out56;
		if ( ( (*( p))) == 46 ) {
			goto _st57;
		}
		goto _st0;
		_st57:
		p+= 1;
		st_case_57:
		if ( p == pe )
			goto _out57;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st67;
			}
			case 50: {
				goto _st69;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st58;
		}
		goto _st0;
		_st58:
		p+= 1;
		st_case_58:
		if ( p == pe )
			goto _out58;
		if ( ( (*( p))) == 46 ) {
			goto _st59;
		}
		goto _st0;
		_st59:
		p+= 1;
		st_case_59:
		if ( p == pe )
			goto _out59;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st62;
			}
			case 50: {
				goto _st64;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st60;
		}
		goto _st0;
		_st60:
		p+= 1;
		st_case_60:
		if ( p == pe )
			goto _out60;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		goto _st0;
		_st61:
		p+= 1;
		st_case_61:
		if ( p == pe )
			goto _out61;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 35: {
				goto _ctr49;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st46;
			}
			case 63: {
				goto _ctr52;
			}
		}
		goto _st0;
		_st62:
		p+= 1;
		st_case_62:
		if ( p == pe )
			goto _out62;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st63;
		}
		goto _st0;
		_st63:
		p+= 1;
		st_case_63:
		if ( p == pe )
			goto _out63;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st60;
		}
		goto _st0;
		_st64:
		p+= 1;
		st_case_64:
		if ( p == pe )
			goto _out64;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st65;
			}
			case 53: {
				goto _st66;
			}
			case 93: {
				goto _st61;
			}
		}
		goto _st0;
		_st65:
		p+= 1;
		st_case_65:
		if ( p == pe )
			goto _out65;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st60;
		}
		goto _st0;
		_st66:
		p+= 1;
		st_case_66:
		if ( p == pe )
			goto _out66;
		if ( ( (*( p))) == 48 ) {
			goto _st60;
		}
		goto _st0;
		_st67:
		p+= 1;
		st_case_67:
		if ( p == pe )
			goto _out67;
		if ( ( (*( p))) == 46 ) {
			goto _st59;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st68;
		}
		goto _st0;
		_st68:
		p+= 1;
		st_case_68:
		if ( p == pe )
			goto _out68;
		if ( ( (*( p))) == 46 ) {
			goto _st59;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st58;
		}
		goto _st0;
		_st69:
		p+= 1;
		st_case_69:
		if ( p == pe )
			goto _out69;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st59;
			}
			case 48: {
				goto _st70;
			}
			case 53: {
				goto _st71;
			}
		}
		goto _st0;
		_st70:
		p+= 1;
		st_case_70:
		if ( p == pe )
			goto _out70;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st58;
		}
		goto _st0;
		_st71:
		p+= 1;
		st_case_71:
		if ( p == pe )
			goto _out71;
		if ( ( (*( p))) == 48 ) {
			goto _st58;
		}
		goto _st0;
		_st72:
		p+= 1;
		st_case_72:
		if ( p == pe )
			goto _out72;
		if ( ( (*( p))) == 46 ) {
			goto _st57;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st73;
		}
		goto _st0;
		_st73:
		p+= 1;
		st_case_73:
		if ( p == pe )
			goto _out73;
		if ( ( (*( p))) == 46 ) {
			goto _st57;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st56;
		}
		goto _st0;
		_st74:
		p+= 1;
		st_case_74:
		if ( p == pe )
			goto _out74;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st57;
			}
			case 48: {
				goto _st75;
			}
			case 53: {
				goto _st76;
			}
		}
		goto _st0;
		_st75:
		p+= 1;
		st_case_75:
		if ( p == pe )
			goto _out75;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st56;
		}
		goto _st0;
		_st76:
		p+= 1;
		st_case_76:
		if ( p == pe )
			goto _out76;
		if ( ( (*( p))) == 48 ) {
			goto _st56;
		}
		goto _st0;
		_st77:
		p+= 1;
		st_case_77:
		if ( p == pe )
			goto _out77;
		if ( ( (*( p))) == 58 ) {
			goto _st80;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st78;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st78;
			}
		} else {
			goto _st78;
		}
		goto _st0;
		_st78:
		p+= 1;
		st_case_78:
		if ( p == pe )
			goto _out78;
		if ( ( (*( p))) == 58 ) {
			goto _st80;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st79;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st79;
			}
		} else {
			goto _st79;
		}
		goto _st0;
		_st79:
		p+= 1;
		st_case_79:
		if ( p == pe )
			goto _out79;
		if ( ( (*( p))) == 58 ) {
			goto _st80;
		}
		goto _st0;
		_st80:
		p+= 1;
		st_case_80:
		if ( p == pe )
			goto _out80;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st81;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st81;
			}
		} else {
			goto _st81;
		}
		goto _st0;
		_st81:
		p+= 1;
		st_case_81:
		if ( p == pe )
			goto _out81;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st82;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st82;
			}
		} else {
			goto _st82;
		}
		goto _st0;
		_st82:
		p+= 1;
		st_case_82:
		if ( p == pe )
			goto _out82;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st83;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st83;
			}
		} else {
			goto _st83;
		}
		goto _st0;
		_st83:
		p+= 1;
		st_case_83:
		if ( p == pe )
			goto _out83;
		if ( ( (*( p))) == 93 ) {
			goto _st61;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st60;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st60;
			}
		} else {
			goto _st60;
		}
		goto _st0;
		_st84:
		p+= 1;
		st_case_84:
		if ( p == pe )
			goto _out84;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st85;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st77;
			}
		} else {
			goto _st77;
		}
		goto _st0;
		_st85:
		p+= 1;
		st_case_85:
		if ( p == pe )
			goto _out85;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st86;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st78;
			}
		} else {
			goto _st78;
		}
		goto _st0;
		_st86:
		p+= 1;
		st_case_86:
		if ( p == pe )
			goto _out86;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st79;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st79;
			}
		} else {
			goto _st79;
		}
		goto _st0;
		_st87:
		p+= 1;
		st_case_87:
		if ( p == pe )
			goto _out87;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 48: {
				goto _st88;
			}
			case 53: {
				goto _st89;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st77;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st77;
			}
		} else {
			goto _st77;
		}
		goto _st0;
		_st88:
		p+= 1;
		st_case_88:
		if ( p == pe )
			goto _out88;
		if ( ( (*( p))) == 58 ) {
			goto _st80;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st86;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st78;
			}
		} else {
			goto _st78;
		}
		goto _st0;
		_st89:
		p+= 1;
		st_case_89:
		if ( p == pe )
			goto _out89;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st86;
			}
			case 58: {
				goto _st80;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st78;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st78;
			}
		} else {
			goto _st78;
		}
		goto _st0;
		_st90:
		p+= 1;
		st_case_90:
		if ( p == pe )
			goto _out90;
		if ( ( (*( p))) == 58 ) {
			goto _st80;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st77;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st77;
			}
		} else {
			goto _st77;
		}
		goto _st0;
		_st91:
		p+= 1;
		st_case_91:
		if ( p == pe )
			goto _out91;
		if ( ( (*( p))) == 58 ) {
			goto _st95;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st92;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st92;
			}
		} else {
			goto _st92;
		}
		goto _st0;
		_st92:
		p+= 1;
		st_case_92:
		if ( p == pe )
			goto _out92;
		if ( ( (*( p))) == 58 ) {
			goto _st95;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st93;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st93;
			}
		} else {
			goto _st93;
		}
		goto _st0;
		_st93:
		p+= 1;
		st_case_93:
		if ( p == pe )
			goto _out93;
		if ( ( (*( p))) == 58 ) {
			goto _st95;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st94;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st94;
			}
		} else {
			goto _st94;
		}
		goto _st0;
		_st94:
		p+= 1;
		st_case_94:
		if ( p == pe )
			goto _out94;
		if ( ( (*( p))) == 58 ) {
			goto _st95;
		}
		goto _st0;
		_st95:
		p+= 1;
		st_case_95:
		if ( p == pe )
			goto _out95;
		if ( ( (*( p))) == 58 ) {
			goto _st166;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st96;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st96;
			}
		} else {
			goto _st96;
		}
		goto _st0;
		_st96:
		p+= 1;
		st_case_96:
		if ( p == pe )
			goto _out96;
		if ( ( (*( p))) == 58 ) {
			goto _st100;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st97;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st97;
			}
		} else {
			goto _st97;
		}
		goto _st0;
		_st97:
		p+= 1;
		st_case_97:
		if ( p == pe )
			goto _out97;
		if ( ( (*( p))) == 58 ) {
			goto _st100;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st98;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st98;
			}
		} else {
			goto _st98;
		}
		goto _st0;
		_st98:
		p+= 1;
		st_case_98:
		if ( p == pe )
			goto _out98;
		if ( ( (*( p))) == 58 ) {
			goto _st100;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st99;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st99;
			}
		} else {
			goto _st99;
		}
		goto _st0;
		_st99:
		p+= 1;
		st_case_99:
		if ( p == pe )
			goto _out99;
		if ( ( (*( p))) == 58 ) {
			goto _st100;
		}
		goto _st0;
		_st100:
		p+= 1;
		st_case_100:
		if ( p == pe )
			goto _out100;
		if ( ( (*( p))) == 58 ) {
			goto _st165;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st101;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st101;
			}
		} else {
			goto _st101;
		}
		goto _st0;
		_st101:
		p+= 1;
		st_case_101:
		if ( p == pe )
			goto _out101;
		if ( ( (*( p))) == 58 ) {
			goto _st105;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st102;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st102;
			}
		} else {
			goto _st102;
		}
		goto _st0;
		_st102:
		p+= 1;
		st_case_102:
		if ( p == pe )
			goto _out102;
		if ( ( (*( p))) == 58 ) {
			goto _st105;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st103;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st103;
			}
		} else {
			goto _st103;
		}
		goto _st0;
		_st103:
		p+= 1;
		st_case_103:
		if ( p == pe )
			goto _out103;
		if ( ( (*( p))) == 58 ) {
			goto _st105;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st104;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st104;
			}
		} else {
			goto _st104;
		}
		goto _st0;
		_st104:
		p+= 1;
		st_case_104:
		if ( p == pe )
			goto _out104;
		if ( ( (*( p))) == 58 ) {
			goto _st105;
		}
		goto _st0;
		_st105:
		p+= 1;
		st_case_105:
		if ( p == pe )
			goto _out105;
		if ( ( (*( p))) == 58 ) {
			goto _st164;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st106;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st106;
			}
		} else {
			goto _st106;
		}
		goto _st0;
		_st106:
		p+= 1;
		st_case_106:
		if ( p == pe )
			goto _out106;
		if ( ( (*( p))) == 58 ) {
			goto _st110;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st107;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st107;
			}
		} else {
			goto _st107;
		}
		goto _st0;
		_st107:
		p+= 1;
		st_case_107:
		if ( p == pe )
			goto _out107;
		if ( ( (*( p))) == 58 ) {
			goto _st110;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st108;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st108;
			}
		} else {
			goto _st108;
		}
		goto _st0;
		_st108:
		p+= 1;
		st_case_108:
		if ( p == pe )
			goto _out108;
		if ( ( (*( p))) == 58 ) {
			goto _st110;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st109;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st109;
			}
		} else {
			goto _st109;
		}
		goto _st0;
		_st109:
		p+= 1;
		st_case_109:
		if ( p == pe )
			goto _out109;
		if ( ( (*( p))) == 58 ) {
			goto _st110;
		}
		goto _st0;
		_st110:
		p+= 1;
		st_case_110:
		if ( p == pe )
			goto _out110;
		if ( ( (*( p))) == 58 ) {
			goto _st163;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st111;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st111;
			}
		} else {
			goto _st111;
		}
		goto _st0;
		_st111:
		p+= 1;
		st_case_111:
		if ( p == pe )
			goto _out111;
		if ( ( (*( p))) == 58 ) {
			goto _st115;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st112;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st112;
			}
		} else {
			goto _st112;
		}
		goto _st0;
		_st112:
		p+= 1;
		st_case_112:
		if ( p == pe )
			goto _out112;
		if ( ( (*( p))) == 58 ) {
			goto _st115;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st113;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st113;
			}
		} else {
			goto _st113;
		}
		goto _st0;
		_st113:
		p+= 1;
		st_case_113:
		if ( p == pe )
			goto _out113;
		if ( ( (*( p))) == 58 ) {
			goto _st115;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st114;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st114;
			}
		} else {
			goto _st114;
		}
		goto _st0;
		_st114:
		p+= 1;
		st_case_114:
		if ( p == pe )
			goto _out114;
		if ( ( (*( p))) == 58 ) {
			goto _st115;
		}
		goto _st0;
		_st115:
		p+= 1;
		st_case_115:
		if ( p == pe )
			goto _out115;
		if ( ( (*( p))) == 58 ) {
			goto _st151;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st116;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st116;
			}
		} else {
			goto _st116;
		}
		goto _st0;
		_st116:
		p+= 1;
		st_case_116:
		if ( p == pe )
			goto _out116;
		if ( ( (*( p))) == 58 ) {
			goto _st120;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st117;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st117;
			}
		} else {
			goto _st117;
		}
		goto _st0;
		_st117:
		p+= 1;
		st_case_117:
		if ( p == pe )
			goto _out117;
		if ( ( (*( p))) == 58 ) {
			goto _st120;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st118;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st118;
			}
		} else {
			goto _st118;
		}
		goto _st0;
		_st118:
		p+= 1;
		st_case_118:
		if ( p == pe )
			goto _out118;
		if ( ( (*( p))) == 58 ) {
			goto _st120;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st119;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st119;
			}
		} else {
			goto _st119;
		}
		goto _st0;
		_st119:
		p+= 1;
		st_case_119:
		if ( p == pe )
			goto _out119;
		if ( ( (*( p))) == 58 ) {
			goto _st120;
		}
		goto _st0;
		_st120:
		p+= 1;
		st_case_120:
		if ( p == pe )
			goto _out120;
		if ( ( (*( p))) == 58 ) {
			goto _st150;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st121;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st121;
			}
		} else {
			goto _st121;
		}
		goto _st0;
		_st121:
		p+= 1;
		st_case_121:
		if ( p == pe )
			goto _out121;
		if ( ( (*( p))) == 58 ) {
			goto _st125;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st122;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st122;
			}
		} else {
			goto _st122;
		}
		goto _st0;
		_st122:
		p+= 1;
		st_case_122:
		if ( p == pe )
			goto _out122;
		if ( ( (*( p))) == 58 ) {
			goto _st125;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st123;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st123;
			}
		} else {
			goto _st123;
		}
		goto _st0;
		_st123:
		p+= 1;
		st_case_123:
		if ( p == pe )
			goto _out123;
		if ( ( (*( p))) == 58 ) {
			goto _st125;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st124;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st124;
			}
		} else {
			goto _st124;
		}
		goto _st0;
		_st124:
		p+= 1;
		st_case_124:
		if ( p == pe )
			goto _out124;
		if ( ( (*( p))) == 58 ) {
			goto _st125;
		}
		goto _st0;
		_st125:
		p+= 1;
		st_case_125:
		if ( p == pe )
			goto _out125;
		if ( ( (*( p))) == 58 ) {
			goto _st126;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st121;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st121;
			}
		} else {
			goto _st121;
		}
		goto _st0;
		_st126:
		p+= 1;
		st_case_126:
		if ( p == pe )
			goto _out126;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st143;
			}
			case 50: {
				goto _st146;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st48;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st149;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st149;
			}
		} else {
			goto _st127;
		}
		goto _st0;
		_st127:
		p+= 1;
		st_case_127:
		if ( p == pe )
			goto _out127;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st128;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st128;
			}
		} else {
			goto _st128;
		}
		goto _st0;
		_st128:
		p+= 1;
		st_case_128:
		if ( p == pe )
			goto _out128;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st129;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st129;
			}
		} else {
			goto _st129;
		}
		goto _st0;
		_st129:
		p+= 1;
		st_case_129:
		if ( p == pe )
			goto _out129;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st130;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st130;
			}
		} else {
			goto _st130;
		}
		goto _st0;
		_st130:
		p+= 1;
		st_case_130:
		if ( p == pe )
			goto _out130;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		goto _st0;
		_st131:
		p+= 1;
		st_case_131:
		if ( p == pe )
			goto _out131;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st136;
			}
			case 50: {
				goto _st139;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st132;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st142;
			}
		} else {
			goto _st142;
		}
		goto _st0;
		_st132:
		p+= 1;
		st_case_132:
		if ( p == pe )
			goto _out132;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st133;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st133;
			}
		} else {
			goto _st133;
		}
		goto _st0;
		_st133:
		p+= 1;
		st_case_133:
		if ( p == pe )
			goto _out133;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st134;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st134;
			}
		} else {
			goto _st134;
		}
		goto _st0;
		_st134:
		p+= 1;
		st_case_134:
		if ( p == pe )
			goto _out134;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st135;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st135;
			}
		} else {
			goto _st135;
		}
		goto _st0;
		_st135:
		p+= 1;
		st_case_135:
		if ( p == pe )
			goto _out135;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		goto _st0;
		_st136:
		p+= 1;
		st_case_136:
		if ( p == pe )
			goto _out136;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st137;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st133;
			}
		} else {
			goto _st133;
		}
		goto _st0;
		_st137:
		p+= 1;
		st_case_137:
		if ( p == pe )
			goto _out137;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st138;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st134;
			}
		} else {
			goto _st134;
		}
		goto _st0;
		_st138:
		p+= 1;
		st_case_138:
		if ( p == pe )
			goto _out138;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st135;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st135;
			}
		} else {
			goto _st135;
		}
		goto _st0;
		_st139:
		p+= 1;
		st_case_139:
		if ( p == pe )
			goto _out139;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 48: {
				goto _st140;
			}
			case 53: {
				goto _st141;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st133;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st133;
			}
		} else {
			goto _st133;
		}
		goto _st0;
		_st140:
		p+= 1;
		st_case_140:
		if ( p == pe )
			goto _out140;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st138;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st134;
			}
		} else {
			goto _st134;
		}
		goto _st0;
		_st141:
		p+= 1;
		st_case_141:
		if ( p == pe )
			goto _out141;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st138;
			}
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st134;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st134;
			}
		} else {
			goto _st134;
		}
		goto _st0;
		_st142:
		p+= 1;
		st_case_142:
		if ( p == pe )
			goto _out142;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st80;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st133;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st133;
			}
		} else {
			goto _st133;
		}
		goto _st0;
		_st143:
		p+= 1;
		st_case_143:
		if ( p == pe )
			goto _out143;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st144;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st128;
			}
		} else {
			goto _st128;
		}
		goto _st0;
		_st144:
		p+= 1;
		st_case_144:
		if ( p == pe )
			goto _out144;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st145;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st129;
			}
		} else {
			goto _st129;
		}
		goto _st0;
		_st145:
		p+= 1;
		st_case_145:
		if ( p == pe )
			goto _out145;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st130;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st130;
			}
		} else {
			goto _st130;
		}
		goto _st0;
		_st146:
		p+= 1;
		st_case_146:
		if ( p == pe )
			goto _out146;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 48: {
				goto _st147;
			}
			case 53: {
				goto _st148;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st128;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st128;
			}
		} else {
			goto _st128;
		}
		goto _st0;
		_st147:
		p+= 1;
		st_case_147:
		if ( p == pe )
			goto _out147;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st145;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st129;
			}
		} else {
			goto _st129;
		}
		goto _st0;
		_st148:
		p+= 1;
		st_case_148:
		if ( p == pe )
			goto _out148;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st145;
			}
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st129;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st129;
			}
		} else {
			goto _st129;
		}
		goto _st0;
		_st149:
		p+= 1;
		st_case_149:
		if ( p == pe )
			goto _out149;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st131;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st128;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st128;
			}
		} else {
			goto _st128;
		}
		goto _st0;
		_st150:
		p+= 1;
		st_case_150:
		if ( p == pe )
			goto _out150;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st143;
			}
			case 50: {
				goto _st146;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st48;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st149;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st149;
			}
		} else {
			goto _st127;
		}
		goto _st0;
		_st151:
		p+= 1;
		st_case_151:
		if ( p == pe )
			goto _out151;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st156;
			}
			case 50: {
				goto _st159;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st48;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st162;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st162;
			}
		} else {
			goto _st152;
		}
		goto _st0;
		_st152:
		p+= 1;
		st_case_152:
		if ( p == pe )
			goto _out152;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st153;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st153;
			}
		} else {
			goto _st153;
		}
		goto _st0;
		_st153:
		p+= 1;
		st_case_153:
		if ( p == pe )
			goto _out153;
		if ( ( (*( p))) == 58 ) {
			goto _st131;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st154;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st154;
			}
		} else {
			goto _st154;
		}
		goto _st0;
		_st154:
		p+= 1;
		st_case_154:
		if ( p == pe )
			goto _out154;
		if ( ( (*( p))) == 58 ) {
			goto _st131;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st155;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st155;
			}
		} else {
			goto _st155;
		}
		goto _st0;
		_st155:
		p+= 1;
		st_case_155:
		if ( p == pe )
			goto _out155;
		if ( ( (*( p))) == 58 ) {
			goto _st131;
		}
		goto _st0;
		_st156:
		p+= 1;
		st_case_156:
		if ( p == pe )
			goto _out156;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st157;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st153;
			}
		} else {
			goto _st153;
		}
		goto _st0;
		_st157:
		p+= 1;
		st_case_157:
		if ( p == pe )
			goto _out157;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st158;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st154;
			}
		} else {
			goto _st154;
		}
		goto _st0;
		_st158:
		p+= 1;
		st_case_158:
		if ( p == pe )
			goto _out158;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st155;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st155;
			}
		} else {
			goto _st155;
		}
		goto _st0;
		_st159:
		p+= 1;
		st_case_159:
		if ( p == pe )
			goto _out159;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st55;
			}
			case 48: {
				goto _st160;
			}
			case 53: {
				goto _st161;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st153;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st153;
			}
		} else {
			goto _st153;
		}
		goto _st0;
		_st160:
		p+= 1;
		st_case_160:
		if ( p == pe )
			goto _out160;
		if ( ( (*( p))) == 58 ) {
			goto _st131;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st158;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st154;
			}
		} else {
			goto _st154;
		}
		goto _st0;
		_st161:
		p+= 1;
		st_case_161:
		if ( p == pe )
			goto _out161;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st158;
			}
			case 58: {
				goto _st131;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st154;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st154;
			}
		} else {
			goto _st154;
		}
		goto _st0;
		_st162:
		p+= 1;
		st_case_162:
		if ( p == pe )
			goto _out162;
		if ( ( (*( p))) == 58 ) {
			goto _st131;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st153;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st153;
			}
		} else {
			goto _st153;
		}
		goto _st0;
		_st163:
		p+= 1;
		st_case_163:
		if ( p == pe )
			goto _out163;
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st48;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st49;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st49;
			}
		} else {
			goto _st49;
		}
		goto _st0;
		_st164:
		p+= 1;
		st_case_164:
		if ( p == pe )
			goto _out164;
		if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
			goto _st48;
		}
		goto _st0;
		_st165:
		p+= 1;
		st_case_165:
		if ( p == pe )
			goto _out165;
		if ( ( (*( p))) == 3 ) {
			goto _st48;
		}
		goto _st0;
		_st166:
		p+= 1;
		st_case_166:
		if ( p == pe )
			goto _out166;
		if ( ( (*( p))) == 4 ) {
			goto _st48;
		}
		goto _st0;
		_st167:
		p+= 1;
		st_case_167:
		if ( p == pe )
			goto _out167;
		if ( ( (*( p))) == 58 ) {
			goto _st168;
		}
		goto _st0;
		_st168:
		p+= 1;
		st_case_168:
		if ( p == pe )
			goto _out168;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st143;
			}
			case 50: {
				goto _st146;
			}
			case 93: {
				goto _st61;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 5 ) {
				goto _st48;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st149;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st149;
			}
		} else {
			goto _st127;
		}
		goto _st0;
		_st169:
		p+= 1;
		st_case_169:
		if ( p == pe )
			goto _out169;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st170;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st170;
			}
		} else {
			goto _st170;
		}
		goto _st0;
		_st170:
		p+= 1;
		st_case_170:
		if ( p == pe )
			goto _out170;
		if ( ( (*( p))) == 46 ) {
			goto _st171;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st170;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st170;
			}
		} else {
			goto _st170;
		}
		goto _st0;
		_st171:
		p+= 1;
		st_case_171:
		if ( p == pe )
			goto _out171;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st172;
			}
			case 36: {
				goto _st172;
			}
			case 61: {
				goto _st172;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5451 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st172;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5465 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5476 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st172;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 38 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st172;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st172;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st172;
			}
		} else {
			goto _st172;
		}
		goto _st0;
		_st172:
		p+= 1;
		st_case_172:
		if ( p == pe )
			goto _out172;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st172;
			}
			case 36: {
				goto _st172;
			}
			case 61: {
				goto _st172;
			}
			case 93: {
				goto _st61;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5525 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st172;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5539 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5550 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st172;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st172;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 38 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st172;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st172;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st172;
			}
		} else {
			goto _st172;
		}
		goto _st0;
		_ctr15:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 5583 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st173;
		_st173:
		p+= 1;
		st_case_173:
		if ( p == pe )
			goto _out173;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr48;
			}
			case 33: {
				goto _st19;
			}
			case 35: {
				goto _ctr49;
			}
			case 37: {
				goto _st24;
			}
			case 43: {
				goto _st173;
			}
			case 47: {
				goto _st26;
			}
			case 58: {
				goto _st174;
			}
			case 59: {
				goto _st19;
			}
			case 61: {
				goto _st19;
			}
			case 63: {
				goto _ctr52;
			}
			case 64: {
				goto _st19;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5627 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st19;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5641 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5652 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st19;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st19;
			}
		}
		if ( ( (*( p))) < 45 ) {
			if ( 36 <= ( (*( p))) ) {
				goto _st19;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st173;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st173;
			}
		} else {
			goto _st173;
		}
		goto _st0;
		_st174:
		p+= 1;
		st_case_174:
		if ( p == pe )
			goto _out174;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr9;
			}
			case 34: {
				goto _st0;
			}
			case 35: {
				goto _ctr11;
			}
			case 37: {
				goto _ctr227;
			}
			case 47: {
				goto _ctr13;
			}
			case 60: {
				goto _st0;
			}
			case 62: {
				goto _st0;
			}
			case 63: {
				goto _ctr14;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5713 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr228;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 91 ) {
			if ( 0 <= ( (*( p))) && ( (*( p))) <= 31 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 93 ) {
			if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 5739 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _ctr228;
				}
				goto _st0;
				
			}
		} else {
			goto _st0;
		}
		goto _ctr226;
		_st175:
		p+= 1;
		st_case_175:
		if ( p == pe )
			goto _out175;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st176;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st176;
		}
		goto _st0;
		_st176:
		p+= 1;
		st_case_176:
		if ( p == pe )
			goto _out176;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st177;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st177;
		}
		goto _st0;
		_st177:
		p+= 1;
		st_case_177:
		if ( p == pe )
			goto _out177;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st178;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st178;
		}
		goto _st0;
		_st178:
		p+= 1;
		st_case_178:
		if ( p == pe )
			goto _out178;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st179;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st179;
		}
		goto _st0;
		_st179:
		p+= 1;
		st_case_179:
		if ( p == pe )
			goto _out179;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st180;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st180;
		}
		goto _st0;
		_st180:
		p+= 1;
		st_case_180:
		if ( p == pe )
			goto _out180;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st181;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st181;
		}
		goto _st0;
		_st181:
		p+= 1;
		st_case_181:
		if ( p == pe )
			goto _out181;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st182;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st182;
		}
		goto _st0;
		_st182:
		p+= 1;
		st_case_182:
		if ( p == pe )
			goto _out182;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st183;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st183;
		}
		goto _st0;
		_st183:
		p+= 1;
		st_case_183:
		if ( p == pe )
			goto _out183;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st184;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st184;
		}
		goto _st0;
		_st184:
		p+= 1;
		st_case_184:
		if ( p == pe )
			goto _out184;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st185;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st185;
		}
		goto _st0;
		_st185:
		p+= 1;
		st_case_185:
		if ( p == pe )
			goto _out185;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st186;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st186;
		}
		goto _st0;
		_st186:
		p+= 1;
		st_case_186:
		if ( p == pe )
			goto _out186;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st187;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st187;
		}
		goto _st0;
		_st187:
		p+= 1;
		st_case_187:
		if ( p == pe )
			goto _out187;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st188;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st188;
		}
		goto _st0;
		_st188:
		p+= 1;
		st_case_188:
		if ( p == pe )
			goto _out188;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st189;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st189;
		}
		goto _st0;
		_st189:
		p+= 1;
		st_case_189:
		if ( p == pe )
			goto _out189;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st190;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st190;
		}
		goto _st0;
		_st190:
		p+= 1;
		st_case_190:
		if ( p == pe )
			goto _out190;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st191;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st191;
		}
		goto _st0;
		_st191:
		p+= 1;
		st_case_191:
		if ( p == pe )
			goto _out191;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st192;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st192;
		}
		goto _st0;
		_st192:
		p+= 1;
		st_case_192:
		if ( p == pe )
			goto _out192;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		if ( ( (*( p))) > 57 ) {
			if ( 65 <= ( (*( p))) && ( (*( p))) <= 90 ) {
				goto _st193;
			}
		} else if ( ( (*( p))) >= 48 ) {
			goto _st193;
		}
		goto _st0;
		_st193:
		p+= 1;
		st_case_193:
		if ( p == pe )
			goto _out193;
		if ( ( (*( p))) == 32 ) {
			goto _ctr6;
		}
		goto _st0;
		_ctr3:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 6053 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st194;
		_st194:
		p+= 1;
		st_case_194:
		if ( p == pe )
			goto _out194;
		if ( ( (*( p))) < 48 ) {
			if ( 45 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st195;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st195;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st195;
			}
		} else {
			goto _st195;
		}
		goto _st0;
		_st195:
		p+= 1;
		st_case_195:
		if ( p == pe )
			goto _out195;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr249;
			}
			case 47: {
				goto _ctr249;
			}
			case 62: {
				goto _ctr249;
			}
		}
		if ( ( (*( p))) < 45 ) {
			if ( 9 <= ( (*( p))) && ( (*( p))) <= 13 ) {
				goto _ctr249;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st195;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st195;
			}
		} else {
			goto _st195;
		}
		goto _st0;
		_ctr249:
			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 6116 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st196;
		_st196:
		p+= 1;
		st_case_196:
		if ( p == pe )
			goto _out196;
		if ( ( (*( p))) == 62 ) {
			goto _st197;
		}
		goto _st196;
		_st197:
		p+= 1;
		st_case_197:
		if ( p == pe )
			goto _out197;
		switch( ( (*( p))) ) {
			case 0: {
				goto _ctr252;
			}
			case 62: {
				goto _st197;
			}
		}
		goto _st196;
		_ctr252:
			{
#line 115 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			parser->xml_sent = 1;
		}
		
#line 6148 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 100 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->xml_sent || parser->json_sent) {
				parser->body_start = PTR_TO(mark) - buffer;
				// +1 includes the \0
				parser->content_len = p - buffer - parser->body_start + 1;
			} else {
				parser->body_start = p - buffer + 1;
				
				if(parser->header_done != NULL) {
					parser->header_done(parser->data, p + 1, pe - p - 1);
				}
			}
			{p+= 1; cs = 349; goto _out;}
		}
		
#line 6166 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st349;
		_ctr263:
			{
#line 119 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			parser->json_sent = 1;
		}
		
#line 6175 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

			{
#line 100 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->xml_sent || parser->json_sent) {
				parser->body_start = PTR_TO(mark) - buffer;
				// +1 includes the \0
				parser->content_len = p - buffer - parser->body_start + 1;
			} else {
				parser->body_start = p - buffer + 1;
				
				if(parser->header_done != NULL) {
					parser->header_done(parser->data, p + 1, pe - p - 1);
				}
			}
			{p+= 1; cs = 349; goto _out;}
		}
		
#line 6193 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st349;
		_st349:
		p+= 1;
		st_case_349:
		if ( p == pe )
			goto _out349;
		goto _st0;
		_ctr4:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 6206 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st198;
		_st198:
		p+= 1;
		st_case_198:
		if ( p == pe )
			goto _out198;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st202;
			}
			case 37: {
				goto _st203;
			}
			case 47: {
				goto _st208;
			}
			case 59: {
				goto _st202;
			}
			case 61: {
				goto _st202;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6235 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st202;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6249 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6260 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st202;
			}
		}
		if ( ( (*( p))) < 64 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st202;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st202;
			}
		} else {
			goto _st202;
		}
		goto _st0;
		_ctr254:
			{
#line 95 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			
			if(parser->request_path != NULL)
			parser->request_path(parser->data, PTR_TO(mark), LEN(mark,p));
		}
		
#line 6292 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st199;
		_st199:
		p+= 1;
		st_case_199:
		if ( p == pe )
			goto _out199;
		if ( ( (*( p))) == 123 ) {
			goto _ctr260;
		}
		goto _st0;
		_ctr260:
			{
#line 53 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"
			MARK(mark, p); }
		
#line 6308 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"

		goto _st200;
		_st200:
		p+= 1;
		st_case_200:
		if ( p == pe )
			goto _out200;
		if ( ( (*( p))) == 125 ) {
			goto _st201;
		}
		goto _st200;
		_st201:
		p+= 1;
		st_case_201:
		if ( p == pe )
			goto _out201;
		switch( ( (*( p))) ) {
			case 0: {
				goto _ctr263;
			}
			case 125: {
				goto _st201;
			}
		}
		goto _st200;
		_st202:
		p+= 1;
		st_case_202:
		if ( p == pe )
			goto _out202;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st202;
			}
			case 37: {
				goto _st203;
			}
			case 47: {
				goto _st205;
			}
			case 59: {
				goto _st202;
			}
			case 61: {
				goto _st202;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6360 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st202;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6374 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6385 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st202;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st202;
			}
		}
		if ( ( (*( p))) < 64 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st202;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st202;
			}
		} else {
			goto _st202;
		}
		goto _st0;
		_st203:
		p+= 1;
		st_case_203:
		if ( p == pe )
			goto _out203;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st204;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st204;
			}
		} else {
			goto _st204;
		}
		goto _st0;
		_st204:
		p+= 1;
		st_case_204:
		if ( p == pe )
			goto _out204;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st202;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st202;
			}
		} else {
			goto _st202;
		}
		goto _st0;
		_st205:
		p+= 1;
		st_case_205:
		if ( p == pe )
			goto _out205;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 37: {
				goto _st206;
			}
			case 60: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6461 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st205;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 62 ) {
			if ( ( (*( p))) > 31 ) {
				if ( 34 <= ( (*( p))) && ( (*( p))) <= 35 ) {
					goto _st0;
				}
			} else if ( ( (*( p))) >= 0 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 63 ) {
			if ( ( (*( p))) > 93 ) {
				if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
					_ck = 0;
					if ( ( parser->uri_relaxed )
#line 6492 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
					_ck += 1;
					if ( 1 <= _ck )
						{goto _st205;
					}
					goto _st0;
					
				}
			} else if ( ( (*( p))) >= 91 ) {
				goto _st0;
			}
		} else {
			goto _st0;
		}
		goto _st205;
		_st206:
		p+= 1;
		st_case_206:
		if ( p == pe )
			goto _out206;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st207;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st207;
			}
		} else {
			goto _st207;
		}
		goto _st0;
		_st207:
		p+= 1;
		st_case_207:
		if ( p == pe )
			goto _out207;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st205;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st205;
			}
		} else {
			goto _st205;
		}
		goto _st0;
		_st208:
		p+= 1;
		st_case_208:
		if ( p == pe )
			goto _out208;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 37: {
				goto _st206;
			}
			case 47: {
				goto _st209;
			}
			case 60: {
				goto _st0;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6562 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st205;
				}
				goto _st0;
				
			}
			case 96: {
				goto _st0;
			}
			case 124: {
				goto _st0;
			}
			case 127: {
				goto _st0;
			}
		}
		if ( ( (*( p))) < 62 ) {
			if ( ( (*( p))) > 31 ) {
				if ( 34 <= ( (*( p))) && ( (*( p))) <= 35 ) {
					goto _st0;
				}
			} else if ( ( (*( p))) >= 0 ) {
				goto _st0;
			}
		} else if ( ( (*( p))) > 63 ) {
			if ( ( (*( p))) > 93 ) {
				if ( 123 <= ( (*( p))) && ( (*( p))) <= 125 ) {
					_ck = 0;
					if ( ( parser->uri_relaxed )
#line 6593 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
					_ck += 1;
					if ( 1 <= _ck )
						{goto _st205;
					}
					goto _st0;
					
				}
			} else if ( ( (*( p))) >= 91 ) {
				goto _st0;
			}
		} else {
			goto _st0;
		}
		goto _st205;
		_st209:
		p+= 1;
		st_case_209:
		if ( p == pe )
			goto _out209;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st210;
			}
			case 37: {
				goto _st211;
			}
			case 47: {
				goto _st205;
			}
			case 58: {
				goto _st213;
			}
			case 61: {
				goto _st210;
			}
			case 64: {
				goto _st217;
			}
			case 91: {
				goto _st222;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6641 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st210;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6655 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6666 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st210;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st210;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st210;
			}
		} else {
			goto _st210;
		}
		goto _st0;
		_st210:
		p+= 1;
		st_case_210:
		if ( p == pe )
			goto _out210;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st210;
			}
			case 37: {
				goto _st211;
			}
			case 47: {
				goto _st205;
			}
			case 58: {
				goto _st213;
			}
			case 61: {
				goto _st210;
			}
			case 64: {
				goto _st217;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6720 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st210;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6734 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6745 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st210;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st210;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st210;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st210;
			}
		} else {
			goto _st210;
		}
		goto _st0;
		_st211:
		p+= 1;
		st_case_211:
		if ( p == pe )
			goto _out211;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st212;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st212;
			}
		} else {
			goto _st212;
		}
		goto _st0;
		_st212:
		p+= 1;
		st_case_212:
		if ( p == pe )
			goto _out212;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st210;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st210;
			}
		} else {
			goto _st210;
		}
		goto _st0;
		_st213:
		p+= 1;
		st_case_213:
		if ( p == pe )
			goto _out213;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st214;
			}
			case 37: {
				goto _st215;
			}
			case 47: {
				goto _st205;
			}
			case 61: {
				goto _st214;
			}
			case 64: {
				goto _st217;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6830 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st214;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6844 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6855 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st214;
			}
		}
		if ( ( (*( p))) < 58 ) {
			if ( ( (*( p))) > 46 ) {
				if ( 48 <= ( (*( p))) ) {
					goto _st213;
				}
			} else if ( ( (*( p))) >= 36 ) {
				goto _st214;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st214;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st214;
			}
		} else {
			goto _st214;
		}
		goto _st0;
		_st214:
		p+= 1;
		st_case_214:
		if ( p == pe )
			goto _out214;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st214;
			}
			case 37: {
				goto _st215;
			}
			case 61: {
				goto _st214;
			}
			case 64: {
				goto _st217;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6908 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st214;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6922 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 6933 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st214;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st214;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st214;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st214;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st214;
			}
		} else {
			goto _st214;
		}
		goto _st0;
		_st215:
		p+= 1;
		st_case_215:
		if ( p == pe )
			goto _out215;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st216;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st216;
			}
		} else {
			goto _st216;
		}
		goto _st0;
		_st216:
		p+= 1;
		st_case_216:
		if ( p == pe )
			goto _out216;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st214;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st214;
			}
		} else {
			goto _st214;
		}
		goto _st0;
		_st217:
		p+= 1;
		st_case_217:
		if ( p == pe )
			goto _out217;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st218;
			}
			case 37: {
				goto _st219;
			}
			case 47: {
				goto _st205;
			}
			case 58: {
				goto _st221;
			}
			case 61: {
				goto _st218;
			}
			case 91: {
				goto _st222;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7025 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st218;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7039 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7050 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st218;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st218;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st218;
			}
		} else {
			goto _st218;
		}
		goto _st0;
		_st218:
		p+= 1;
		st_case_218:
		if ( p == pe )
			goto _out218;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 33: {
				goto _st218;
			}
			case 37: {
				goto _st219;
			}
			case 47: {
				goto _st205;
			}
			case 58: {
				goto _st221;
			}
			case 61: {
				goto _st218;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7101 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st218;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7115 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 7126 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st218;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st218;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 36 <= ( (*( p))) && ( (*( p))) <= 59 ) {
				goto _st218;
			}
		} else if ( ( (*( p))) > 90 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
				goto _st218;
			}
		} else {
			goto _st218;
		}
		goto _st0;
		_st219:
		p+= 1;
		st_case_219:
		if ( p == pe )
			goto _out219;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st220;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st220;
			}
		} else {
			goto _st220;
		}
		goto _st0;
		_st220:
		p+= 1;
		st_case_220:
		if ( p == pe )
			goto _out220;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st218;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st218;
			}
		} else {
			goto _st218;
		}
		goto _st0;
		_st221:
		p+= 1;
		st_case_221:
		if ( p == pe )
			goto _out221;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 47: {
				goto _st205;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st221;
		}
		goto _st0;
		_st222:
		p+= 1;
		st_case_222:
		if ( p == pe )
			goto _out222;
		switch( ( (*( p))) ) {
			case 6: {
				goto _st223;
			}
			case 58: {
				goto _st342;
			}
			case 118: {
				goto _st344;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st266;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st266;
			}
		} else {
			goto _st266;
		}
		goto _st0;
		_st223:
		p+= 1;
		st_case_223:
		if ( p == pe )
			goto _out223;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st224;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st224;
			}
		} else {
			goto _st224;
		}
		goto _st0;
		_st224:
		p+= 1;
		st_case_224:
		if ( p == pe )
			goto _out224;
		if ( ( (*( p))) == 58 ) {
			goto _st228;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st225;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st225;
			}
		} else {
			goto _st225;
		}
		goto _st0;
		_st225:
		p+= 1;
		st_case_225:
		if ( p == pe )
			goto _out225;
		if ( ( (*( p))) == 58 ) {
			goto _st228;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st226;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st226;
			}
		} else {
			goto _st226;
		}
		goto _st0;
		_st226:
		p+= 1;
		st_case_226:
		if ( p == pe )
			goto _out226;
		if ( ( (*( p))) == 58 ) {
			goto _st228;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st227;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st227;
			}
		} else {
			goto _st227;
		}
		goto _st0;
		_st227:
		p+= 1;
		st_case_227:
		if ( p == pe )
			goto _out227;
		if ( ( (*( p))) == 58 ) {
			goto _st228;
		}
		goto _st0;
		_st228:
		p+= 1;
		st_case_228:
		if ( p == pe )
			goto _out228;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st259;
			}
			case 50: {
				goto _st262;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st229;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st265;
			}
		} else {
			goto _st265;
		}
		goto _st0;
		_st229:
		p+= 1;
		st_case_229:
		if ( p == pe )
			goto _out229;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st252;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st252;
			}
		} else {
			goto _st252;
		}
		goto _st0;
		_st230:
		p+= 1;
		st_case_230:
		if ( p == pe )
			goto _out230;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st247;
			}
			case 50: {
				goto _st249;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st231;
		}
		goto _st0;
		_st231:
		p+= 1;
		st_case_231:
		if ( p == pe )
			goto _out231;
		if ( ( (*( p))) == 46 ) {
			goto _st232;
		}
		goto _st0;
		_st232:
		p+= 1;
		st_case_232:
		if ( p == pe )
			goto _out232;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st242;
			}
			case 50: {
				goto _st244;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st233;
		}
		goto _st0;
		_st233:
		p+= 1;
		st_case_233:
		if ( p == pe )
			goto _out233;
		if ( ( (*( p))) == 46 ) {
			goto _st234;
		}
		goto _st0;
		_st234:
		p+= 1;
		st_case_234:
		if ( p == pe )
			goto _out234;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st237;
			}
			case 50: {
				goto _st239;
			}
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st235;
		}
		goto _st0;
		_st235:
		p+= 1;
		st_case_235:
		if ( p == pe )
			goto _out235;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		goto _st0;
		_st236:
		p+= 1;
		st_case_236:
		if ( p == pe )
			goto _out236;
		switch( ( (*( p))) ) {
			case 32: {
				goto _ctr254;
			}
			case 47: {
				goto _st205;
			}
			case 58: {
				goto _st221;
			}
		}
		goto _st0;
		_st237:
		p+= 1;
		st_case_237:
		if ( p == pe )
			goto _out237;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st238;
		}
		goto _st0;
		_st238:
		p+= 1;
		st_case_238:
		if ( p == pe )
			goto _out238;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st235;
		}
		goto _st0;
		_st239:
		p+= 1;
		st_case_239:
		if ( p == pe )
			goto _out239;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st240;
			}
			case 53: {
				goto _st241;
			}
			case 93: {
				goto _st236;
			}
		}
		goto _st0;
		_st240:
		p+= 1;
		st_case_240:
		if ( p == pe )
			goto _out240;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st235;
		}
		goto _st0;
		_st241:
		p+= 1;
		st_case_241:
		if ( p == pe )
			goto _out241;
		if ( ( (*( p))) == 48 ) {
			goto _st235;
		}
		goto _st0;
		_st242:
		p+= 1;
		st_case_242:
		if ( p == pe )
			goto _out242;
		if ( ( (*( p))) == 46 ) {
			goto _st234;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st243;
		}
		goto _st0;
		_st243:
		p+= 1;
		st_case_243:
		if ( p == pe )
			goto _out243;
		if ( ( (*( p))) == 46 ) {
			goto _st234;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st233;
		}
		goto _st0;
		_st244:
		p+= 1;
		st_case_244:
		if ( p == pe )
			goto _out244;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st234;
			}
			case 48: {
				goto _st245;
			}
			case 53: {
				goto _st246;
			}
		}
		goto _st0;
		_st245:
		p+= 1;
		st_case_245:
		if ( p == pe )
			goto _out245;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st233;
		}
		goto _st0;
		_st246:
		p+= 1;
		st_case_246:
		if ( p == pe )
			goto _out246;
		if ( ( (*( p))) == 48 ) {
			goto _st233;
		}
		goto _st0;
		_st247:
		p+= 1;
		st_case_247:
		if ( p == pe )
			goto _out247;
		if ( ( (*( p))) == 46 ) {
			goto _st232;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st248;
		}
		goto _st0;
		_st248:
		p+= 1;
		st_case_248:
		if ( p == pe )
			goto _out248;
		if ( ( (*( p))) == 46 ) {
			goto _st232;
		}
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st231;
		}
		goto _st0;
		_st249:
		p+= 1;
		st_case_249:
		if ( p == pe )
			goto _out249;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st232;
			}
			case 48: {
				goto _st250;
			}
			case 53: {
				goto _st251;
			}
		}
		goto _st0;
		_st250:
		p+= 1;
		st_case_250:
		if ( p == pe )
			goto _out250;
		if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
			goto _st231;
		}
		goto _st0;
		_st251:
		p+= 1;
		st_case_251:
		if ( p == pe )
			goto _out251;
		if ( ( (*( p))) == 48 ) {
			goto _st231;
		}
		goto _st0;
		_st252:
		p+= 1;
		st_case_252:
		if ( p == pe )
			goto _out252;
		if ( ( (*( p))) == 58 ) {
			goto _st255;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st253;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st253;
			}
		} else {
			goto _st253;
		}
		goto _st0;
		_st253:
		p+= 1;
		st_case_253:
		if ( p == pe )
			goto _out253;
		if ( ( (*( p))) == 58 ) {
			goto _st255;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st254;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st254;
			}
		} else {
			goto _st254;
		}
		goto _st0;
		_st254:
		p+= 1;
		st_case_254:
		if ( p == pe )
			goto _out254;
		if ( ( (*( p))) == 58 ) {
			goto _st255;
		}
		goto _st0;
		_st255:
		p+= 1;
		st_case_255:
		if ( p == pe )
			goto _out255;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st256;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st256;
			}
		} else {
			goto _st256;
		}
		goto _st0;
		_st256:
		p+= 1;
		st_case_256:
		if ( p == pe )
			goto _out256;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st257;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st257;
			}
		} else {
			goto _st257;
		}
		goto _st0;
		_st257:
		p+= 1;
		st_case_257:
		if ( p == pe )
			goto _out257;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st258;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st258;
			}
		} else {
			goto _st258;
		}
		goto _st0;
		_st258:
		p+= 1;
		st_case_258:
		if ( p == pe )
			goto _out258;
		if ( ( (*( p))) == 93 ) {
			goto _st236;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st235;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st235;
			}
		} else {
			goto _st235;
		}
		goto _st0;
		_st259:
		p+= 1;
		st_case_259:
		if ( p == pe )
			goto _out259;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st260;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st252;
			}
		} else {
			goto _st252;
		}
		goto _st0;
		_st260:
		p+= 1;
		st_case_260:
		if ( p == pe )
			goto _out260;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st261;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st253;
			}
		} else {
			goto _st253;
		}
		goto _st0;
		_st261:
		p+= 1;
		st_case_261:
		if ( p == pe )
			goto _out261;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st254;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st254;
			}
		} else {
			goto _st254;
		}
		goto _st0;
		_st262:
		p+= 1;
		st_case_262:
		if ( p == pe )
			goto _out262;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 48: {
				goto _st263;
			}
			case 53: {
				goto _st264;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st252;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st252;
			}
		} else {
			goto _st252;
		}
		goto _st0;
		_st263:
		p+= 1;
		st_case_263:
		if ( p == pe )
			goto _out263;
		if ( ( (*( p))) == 58 ) {
			goto _st255;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st261;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st253;
			}
		} else {
			goto _st253;
		}
		goto _st0;
		_st264:
		p+= 1;
		st_case_264:
		if ( p == pe )
			goto _out264;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st261;
			}
			case 58: {
				goto _st255;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st253;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st253;
			}
		} else {
			goto _st253;
		}
		goto _st0;
		_st265:
		p+= 1;
		st_case_265:
		if ( p == pe )
			goto _out265;
		if ( ( (*( p))) == 58 ) {
			goto _st255;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st252;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st252;
			}
		} else {
			goto _st252;
		}
		goto _st0;
		_st266:
		p+= 1;
		st_case_266:
		if ( p == pe )
			goto _out266;
		if ( ( (*( p))) == 58 ) {
			goto _st270;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st267;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st267;
			}
		} else {
			goto _st267;
		}
		goto _st0;
		_st267:
		p+= 1;
		st_case_267:
		if ( p == pe )
			goto _out267;
		if ( ( (*( p))) == 58 ) {
			goto _st270;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st268;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st268;
			}
		} else {
			goto _st268;
		}
		goto _st0;
		_st268:
		p+= 1;
		st_case_268:
		if ( p == pe )
			goto _out268;
		if ( ( (*( p))) == 58 ) {
			goto _st270;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st269;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st269;
			}
		} else {
			goto _st269;
		}
		goto _st0;
		_st269:
		p+= 1;
		st_case_269:
		if ( p == pe )
			goto _out269;
		if ( ( (*( p))) == 58 ) {
			goto _st270;
		}
		goto _st0;
		_st270:
		p+= 1;
		st_case_270:
		if ( p == pe )
			goto _out270;
		if ( ( (*( p))) == 58 ) {
			goto _st341;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st271;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st271;
			}
		} else {
			goto _st271;
		}
		goto _st0;
		_st271:
		p+= 1;
		st_case_271:
		if ( p == pe )
			goto _out271;
		if ( ( (*( p))) == 58 ) {
			goto _st275;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st272;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st272;
			}
		} else {
			goto _st272;
		}
		goto _st0;
		_st272:
		p+= 1;
		st_case_272:
		if ( p == pe )
			goto _out272;
		if ( ( (*( p))) == 58 ) {
			goto _st275;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st273;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st273;
			}
		} else {
			goto _st273;
		}
		goto _st0;
		_st273:
		p+= 1;
		st_case_273:
		if ( p == pe )
			goto _out273;
		if ( ( (*( p))) == 58 ) {
			goto _st275;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st274;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st274;
			}
		} else {
			goto _st274;
		}
		goto _st0;
		_st274:
		p+= 1;
		st_case_274:
		if ( p == pe )
			goto _out274;
		if ( ( (*( p))) == 58 ) {
			goto _st275;
		}
		goto _st0;
		_st275:
		p+= 1;
		st_case_275:
		if ( p == pe )
			goto _out275;
		if ( ( (*( p))) == 58 ) {
			goto _st340;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st276;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st276;
			}
		} else {
			goto _st276;
		}
		goto _st0;
		_st276:
		p+= 1;
		st_case_276:
		if ( p == pe )
			goto _out276;
		if ( ( (*( p))) == 58 ) {
			goto _st280;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st277;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st277;
			}
		} else {
			goto _st277;
		}
		goto _st0;
		_st277:
		p+= 1;
		st_case_277:
		if ( p == pe )
			goto _out277;
		if ( ( (*( p))) == 58 ) {
			goto _st280;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st278;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st278;
			}
		} else {
			goto _st278;
		}
		goto _st0;
		_st278:
		p+= 1;
		st_case_278:
		if ( p == pe )
			goto _out278;
		if ( ( (*( p))) == 58 ) {
			goto _st280;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st279;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st279;
			}
		} else {
			goto _st279;
		}
		goto _st0;
		_st279:
		p+= 1;
		st_case_279:
		if ( p == pe )
			goto _out279;
		if ( ( (*( p))) == 58 ) {
			goto _st280;
		}
		goto _st0;
		_st280:
		p+= 1;
		st_case_280:
		if ( p == pe )
			goto _out280;
		if ( ( (*( p))) == 58 ) {
			goto _st339;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st281;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st281;
			}
		} else {
			goto _st281;
		}
		goto _st0;
		_st281:
		p+= 1;
		st_case_281:
		if ( p == pe )
			goto _out281;
		if ( ( (*( p))) == 58 ) {
			goto _st285;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st282;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st282;
			}
		} else {
			goto _st282;
		}
		goto _st0;
		_st282:
		p+= 1;
		st_case_282:
		if ( p == pe )
			goto _out282;
		if ( ( (*( p))) == 58 ) {
			goto _st285;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st283;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st283;
			}
		} else {
			goto _st283;
		}
		goto _st0;
		_st283:
		p+= 1;
		st_case_283:
		if ( p == pe )
			goto _out283;
		if ( ( (*( p))) == 58 ) {
			goto _st285;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st284;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st284;
			}
		} else {
			goto _st284;
		}
		goto _st0;
		_st284:
		p+= 1;
		st_case_284:
		if ( p == pe )
			goto _out284;
		if ( ( (*( p))) == 58 ) {
			goto _st285;
		}
		goto _st0;
		_st285:
		p+= 1;
		st_case_285:
		if ( p == pe )
			goto _out285;
		if ( ( (*( p))) == 58 ) {
			goto _st338;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st286;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st286;
			}
		} else {
			goto _st286;
		}
		goto _st0;
		_st286:
		p+= 1;
		st_case_286:
		if ( p == pe )
			goto _out286;
		if ( ( (*( p))) == 58 ) {
			goto _st290;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st287;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st287;
			}
		} else {
			goto _st287;
		}
		goto _st0;
		_st287:
		p+= 1;
		st_case_287:
		if ( p == pe )
			goto _out287;
		if ( ( (*( p))) == 58 ) {
			goto _st290;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st288;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st288;
			}
		} else {
			goto _st288;
		}
		goto _st0;
		_st288:
		p+= 1;
		st_case_288:
		if ( p == pe )
			goto _out288;
		if ( ( (*( p))) == 58 ) {
			goto _st290;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st289;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st289;
			}
		} else {
			goto _st289;
		}
		goto _st0;
		_st289:
		p+= 1;
		st_case_289:
		if ( p == pe )
			goto _out289;
		if ( ( (*( p))) == 58 ) {
			goto _st290;
		}
		goto _st0;
		_st290:
		p+= 1;
		st_case_290:
		if ( p == pe )
			goto _out290;
		if ( ( (*( p))) == 58 ) {
			goto _st326;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st291;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st291;
			}
		} else {
			goto _st291;
		}
		goto _st0;
		_st291:
		p+= 1;
		st_case_291:
		if ( p == pe )
			goto _out291;
		if ( ( (*( p))) == 58 ) {
			goto _st295;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st292;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st292;
			}
		} else {
			goto _st292;
		}
		goto _st0;
		_st292:
		p+= 1;
		st_case_292:
		if ( p == pe )
			goto _out292;
		if ( ( (*( p))) == 58 ) {
			goto _st295;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st293;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st293;
			}
		} else {
			goto _st293;
		}
		goto _st0;
		_st293:
		p+= 1;
		st_case_293:
		if ( p == pe )
			goto _out293;
		if ( ( (*( p))) == 58 ) {
			goto _st295;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st294;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st294;
			}
		} else {
			goto _st294;
		}
		goto _st0;
		_st294:
		p+= 1;
		st_case_294:
		if ( p == pe )
			goto _out294;
		if ( ( (*( p))) == 58 ) {
			goto _st295;
		}
		goto _st0;
		_st295:
		p+= 1;
		st_case_295:
		if ( p == pe )
			goto _out295;
		if ( ( (*( p))) == 58 ) {
			goto _st325;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st296;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st296;
			}
		} else {
			goto _st296;
		}
		goto _st0;
		_st296:
		p+= 1;
		st_case_296:
		if ( p == pe )
			goto _out296;
		if ( ( (*( p))) == 58 ) {
			goto _st300;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st297;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st297;
			}
		} else {
			goto _st297;
		}
		goto _st0;
		_st297:
		p+= 1;
		st_case_297:
		if ( p == pe )
			goto _out297;
		if ( ( (*( p))) == 58 ) {
			goto _st300;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st298;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st298;
			}
		} else {
			goto _st298;
		}
		goto _st0;
		_st298:
		p+= 1;
		st_case_298:
		if ( p == pe )
			goto _out298;
		if ( ( (*( p))) == 58 ) {
			goto _st300;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st299;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st299;
			}
		} else {
			goto _st299;
		}
		goto _st0;
		_st299:
		p+= 1;
		st_case_299:
		if ( p == pe )
			goto _out299;
		if ( ( (*( p))) == 58 ) {
			goto _st300;
		}
		goto _st0;
		_st300:
		p+= 1;
		st_case_300:
		if ( p == pe )
			goto _out300;
		if ( ( (*( p))) == 58 ) {
			goto _st301;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st296;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st296;
			}
		} else {
			goto _st296;
		}
		goto _st0;
		_st301:
		p+= 1;
		st_case_301:
		if ( p == pe )
			goto _out301;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st318;
			}
			case 50: {
				goto _st321;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st223;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st324;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st324;
			}
		} else {
			goto _st302;
		}
		goto _st0;
		_st302:
		p+= 1;
		st_case_302:
		if ( p == pe )
			goto _out302;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st303;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st303;
			}
		} else {
			goto _st303;
		}
		goto _st0;
		_st303:
		p+= 1;
		st_case_303:
		if ( p == pe )
			goto _out303;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st304;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st304;
			}
		} else {
			goto _st304;
		}
		goto _st0;
		_st304:
		p+= 1;
		st_case_304:
		if ( p == pe )
			goto _out304;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st305;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st305;
			}
		} else {
			goto _st305;
		}
		goto _st0;
		_st305:
		p+= 1;
		st_case_305:
		if ( p == pe )
			goto _out305;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		goto _st0;
		_st306:
		p+= 1;
		st_case_306:
		if ( p == pe )
			goto _out306;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st311;
			}
			case 50: {
				goto _st314;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st307;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st317;
			}
		} else {
			goto _st317;
		}
		goto _st0;
		_st307:
		p+= 1;
		st_case_307:
		if ( p == pe )
			goto _out307;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st308;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st308;
			}
		} else {
			goto _st308;
		}
		goto _st0;
		_st308:
		p+= 1;
		st_case_308:
		if ( p == pe )
			goto _out308;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st309;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st309;
			}
		} else {
			goto _st309;
		}
		goto _st0;
		_st309:
		p+= 1;
		st_case_309:
		if ( p == pe )
			goto _out309;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st310;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st310;
			}
		} else {
			goto _st310;
		}
		goto _st0;
		_st310:
		p+= 1;
		st_case_310:
		if ( p == pe )
			goto _out310;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		goto _st0;
		_st311:
		p+= 1;
		st_case_311:
		if ( p == pe )
			goto _out311;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st312;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st308;
			}
		} else {
			goto _st308;
		}
		goto _st0;
		_st312:
		p+= 1;
		st_case_312:
		if ( p == pe )
			goto _out312;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st313;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st309;
			}
		} else {
			goto _st309;
		}
		goto _st0;
		_st313:
		p+= 1;
		st_case_313:
		if ( p == pe )
			goto _out313;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st310;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st310;
			}
		} else {
			goto _st310;
		}
		goto _st0;
		_st314:
		p+= 1;
		st_case_314:
		if ( p == pe )
			goto _out314;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 48: {
				goto _st315;
			}
			case 53: {
				goto _st316;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st308;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st308;
			}
		} else {
			goto _st308;
		}
		goto _st0;
		_st315:
		p+= 1;
		st_case_315:
		if ( p == pe )
			goto _out315;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st313;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st309;
			}
		} else {
			goto _st309;
		}
		goto _st0;
		_st316:
		p+= 1;
		st_case_316:
		if ( p == pe )
			goto _out316;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st313;
			}
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st309;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st309;
			}
		} else {
			goto _st309;
		}
		goto _st0;
		_st317:
		p+= 1;
		st_case_317:
		if ( p == pe )
			goto _out317;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st255;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st308;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st308;
			}
		} else {
			goto _st308;
		}
		goto _st0;
		_st318:
		p+= 1;
		st_case_318:
		if ( p == pe )
			goto _out318;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st319;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st303;
			}
		} else {
			goto _st303;
		}
		goto _st0;
		_st319:
		p+= 1;
		st_case_319:
		if ( p == pe )
			goto _out319;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st320;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st304;
			}
		} else {
			goto _st304;
		}
		goto _st0;
		_st320:
		p+= 1;
		st_case_320:
		if ( p == pe )
			goto _out320;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st305;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st305;
			}
		} else {
			goto _st305;
		}
		goto _st0;
		_st321:
		p+= 1;
		st_case_321:
		if ( p == pe )
			goto _out321;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 48: {
				goto _st322;
			}
			case 53: {
				goto _st323;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st303;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st303;
			}
		} else {
			goto _st303;
		}
		goto _st0;
		_st322:
		p+= 1;
		st_case_322:
		if ( p == pe )
			goto _out322;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st320;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st304;
			}
		} else {
			goto _st304;
		}
		goto _st0;
		_st323:
		p+= 1;
		st_case_323:
		if ( p == pe )
			goto _out323;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st320;
			}
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st304;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st304;
			}
		} else {
			goto _st304;
		}
		goto _st0;
		_st324:
		p+= 1;
		st_case_324:
		if ( p == pe )
			goto _out324;
		switch( ( (*( p))) ) {
			case 58: {
				goto _st306;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st303;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st303;
			}
		} else {
			goto _st303;
		}
		goto _st0;
		_st325:
		p+= 1;
		st_case_325:
		if ( p == pe )
			goto _out325;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st318;
			}
			case 50: {
				goto _st321;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st223;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st324;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st324;
			}
		} else {
			goto _st302;
		}
		goto _st0;
		_st326:
		p+= 1;
		st_case_326:
		if ( p == pe )
			goto _out326;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st331;
			}
			case 50: {
				goto _st334;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st223;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st337;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st337;
			}
		} else {
			goto _st327;
		}
		goto _st0;
		_st327:
		p+= 1;
		st_case_327:
		if ( p == pe )
			goto _out327;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st328;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st328;
			}
		} else {
			goto _st328;
		}
		goto _st0;
		_st328:
		p+= 1;
		st_case_328:
		if ( p == pe )
			goto _out328;
		if ( ( (*( p))) == 58 ) {
			goto _st306;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st329;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st329;
			}
		} else {
			goto _st329;
		}
		goto _st0;
		_st329:
		p+= 1;
		st_case_329:
		if ( p == pe )
			goto _out329;
		if ( ( (*( p))) == 58 ) {
			goto _st306;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st330;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st330;
			}
		} else {
			goto _st330;
		}
		goto _st0;
		_st330:
		p+= 1;
		st_case_330:
		if ( p == pe )
			goto _out330;
		if ( ( (*( p))) == 58 ) {
			goto _st306;
		}
		goto _st0;
		_st331:
		p+= 1;
		st_case_331:
		if ( p == pe )
			goto _out331;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st332;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st328;
			}
		} else {
			goto _st328;
		}
		goto _st0;
		_st332:
		p+= 1;
		st_case_332:
		if ( p == pe )
			goto _out332;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st333;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st329;
			}
		} else {
			goto _st329;
		}
		goto _st0;
		_st333:
		p+= 1;
		st_case_333:
		if ( p == pe )
			goto _out333;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st330;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st330;
			}
		} else {
			goto _st330;
		}
		goto _st0;
		_st334:
		p+= 1;
		st_case_334:
		if ( p == pe )
			goto _out334;
		switch( ( (*( p))) ) {
			case 46: {
				goto _st230;
			}
			case 48: {
				goto _st335;
			}
			case 53: {
				goto _st336;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st328;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st328;
			}
		} else {
			goto _st328;
		}
		goto _st0;
		_st335:
		p+= 1;
		st_case_335:
		if ( p == pe )
			goto _out335;
		if ( ( (*( p))) == 58 ) {
			goto _st306;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st333;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st329;
			}
		} else {
			goto _st329;
		}
		goto _st0;
		_st336:
		p+= 1;
		st_case_336:
		if ( p == pe )
			goto _out336;
		switch( ( (*( p))) ) {
			case 48: {
				goto _st333;
			}
			case 58: {
				goto _st306;
			}
		}
		if ( ( (*( p))) < 65 ) {
			if ( 49 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st329;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st329;
			}
		} else {
			goto _st329;
		}
		goto _st0;
		_st337:
		p+= 1;
		st_case_337:
		if ( p == pe )
			goto _out337;
		if ( ( (*( p))) == 58 ) {
			goto _st306;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st328;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st328;
			}
		} else {
			goto _st328;
		}
		goto _st0;
		_st338:
		p+= 1;
		st_case_338:
		if ( p == pe )
			goto _out338;
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
				goto _st223;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st224;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st224;
			}
		} else {
			goto _st224;
		}
		goto _st0;
		_st339:
		p+= 1;
		st_case_339:
		if ( p == pe )
			goto _out339;
		if ( 2 <= ( (*( p))) && ( (*( p))) <= 3 ) {
			goto _st223;
		}
		goto _st0;
		_st340:
		p+= 1;
		st_case_340:
		if ( p == pe )
			goto _out340;
		if ( ( (*( p))) == 3 ) {
			goto _st223;
		}
		goto _st0;
		_st341:
		p+= 1;
		st_case_341:
		if ( p == pe )
			goto _out341;
		if ( ( (*( p))) == 4 ) {
			goto _st223;
		}
		goto _st0;
		_st342:
		p+= 1;
		st_case_342:
		if ( p == pe )
			goto _out342;
		if ( ( (*( p))) == 58 ) {
			goto _st343;
		}
		goto _st0;
		_st343:
		p+= 1;
		st_case_343:
		if ( p == pe )
			goto _out343;
		switch( ( (*( p))) ) {
			case 49: {
				goto _st318;
			}
			case 50: {
				goto _st321;
			}
			case 93: {
				goto _st236;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 2 <= ( (*( p))) && ( (*( p))) <= 5 ) {
				goto _st223;
			}
		} else if ( ( (*( p))) > 57 ) {
			if ( ( (*( p))) > 70 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
					goto _st324;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st324;
			}
		} else {
			goto _st302;
		}
		goto _st0;
		_st344:
		p+= 1;
		st_case_344:
		if ( p == pe )
			goto _out344;
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st345;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st345;
			}
		} else {
			goto _st345;
		}
		goto _st0;
		_st345:
		p+= 1;
		st_case_345:
		if ( p == pe )
			goto _out345;
		if ( ( (*( p))) == 46 ) {
			goto _st346;
		}
		if ( ( (*( p))) < 65 ) {
			if ( 48 <= ( (*( p))) && ( (*( p))) <= 57 ) {
				goto _st345;
			}
		} else if ( ( (*( p))) > 70 ) {
			if ( 97 <= ( (*( p))) && ( (*( p))) <= 102 ) {
				goto _st345;
			}
		} else {
			goto _st345;
		}
		goto _st0;
		_st346:
		p+= 1;
		st_case_346:
		if ( p == pe )
			goto _out346;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st347;
			}
			case 36: {
				goto _st347;
			}
			case 61: {
				goto _st347;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9637 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st347;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9651 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9662 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st347;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 38 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st347;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st347;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st347;
			}
		} else {
			goto _st347;
		}
		goto _st0;
		_st347:
		p+= 1;
		st_case_347:
		if ( p == pe )
			goto _out347;
		switch( ( (*( p))) ) {
			case 33: {
				goto _st347;
			}
			case 36: {
				goto _st347;
			}
			case 61: {
				goto _st347;
			}
			case 93: {
				goto _st236;
			}
			case 94: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9711 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 95: {
				goto _st347;
			}
			case 123: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9725 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 125: {
				_ck = 0;
				if ( ( parser->uri_relaxed )
#line 9736 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl.cc"
 )
				_ck += 1;
				if ( 1 <= _ck )
					{goto _st347;
				}
				goto _st0;
				
			}
			case 126: {
				goto _st347;
			}
		}
		if ( ( (*( p))) < 48 ) {
			if ( 38 <= ( (*( p))) && ( (*( p))) <= 46 ) {
				goto _st347;
			}
		} else if ( ( (*( p))) > 59 ) {
			if ( ( (*( p))) > 90 ) {
				if ( 97 <= ( (*( p))) && ( (*( p))) <= 122 ) {
					goto _st347;
				}
			} else if ( ( (*( p))) >= 65 ) {
				goto _st347;
			}
		} else {
			goto _st347;
		}
		goto _st0;
		_out1: cs = 1; goto _out; 
		_out0: cs = 0; goto _out; 
		_out2: cs = 2; goto _out; 
		_out3: cs = 3; goto _out; 
		_out4: cs = 4; goto _out; 
		_out5: cs = 5; goto _out; 
		_out6: cs = 6; goto _out; 
		_out7: cs = 7; goto _out; 
		_out8: cs = 8; goto _out; 
		_out9: cs = 9; goto _out; 
		_out10: cs = 10; goto _out; 
		_out11: cs = 11; goto _out; 
		_out12: cs = 12; goto _out; 
		_out13: cs = 13; goto _out; 
		_out14: cs = 14; goto _out; 
		_out15: cs = 15; goto _out; 
		_out16: cs = 16; goto _out; 
		_out17: cs = 17; goto _out; 
		_out348: cs = 348; goto _out; 
		_out18: cs = 18; goto _out; 
		_out19: cs = 19; goto _out; 
		_out20: cs = 20; goto _out; 
		_out21: cs = 21; goto _out; 
		_out22: cs = 22; goto _out; 
		_out23: cs = 23; goto _out; 
		_out24: cs = 24; goto _out; 
		_out25: cs = 25; goto _out; 
		_out26: cs = 26; goto _out; 
		_out27: cs = 27; goto _out; 
		_out28: cs = 28; goto _out; 
		_out29: cs = 29; goto _out; 
		_out30: cs = 30; goto _out; 
		_out31: cs = 31; goto _out; 
		_out32: cs = 32; goto _out; 
		_out33: cs = 33; goto _out; 
		_out34: cs = 34; goto _out; 
		_out35: cs = 35; goto _out; 
		_out36: cs = 36; goto _out; 
		_out37: cs = 37; goto _out; 
		_out38: cs = 38; goto _out; 
		_out39: cs = 39; goto _out; 
		_out40: cs = 40; goto _out; 
		_out41: cs = 41; goto _out; 
		_out42: cs = 42; goto _out; 
		_out43: cs = 43; goto _out; 
		_out44: cs = 44; goto _out; 
		_out45: cs = 45; goto _out; 
		_out46: cs = 46; goto _out; 
		_out47: cs = 47; goto _out; 
		_out48: cs = 48; goto _out; 
		_out49: cs = 49; goto _out; 
		_out50: cs = 50; goto _out; 
		_out51: cs = 51; goto _out; 
		_out52: cs = 52; goto _out; 
		_out53: cs = 53; goto _out; 
		_out54: cs = 54; goto _out; 
		_out55: cs = 55; goto _out; 
		_out56: cs = 56; goto _out; 
		_out57: cs = 57; goto _out; 
		_out58: cs = 58; goto _out; 
		_out59: cs = 59; goto _out; 
		_out60: cs = 60; goto _out; 
		_out61: cs = 61; goto _out; 
		_out62: cs = 62; goto _out; 
		_out63: cs = 63; goto _out; 
		_out64: cs = 64; goto _out; 
		_out65: cs = 65; goto _out; 
		_out66: cs = 66; goto _out; 
		_out67: cs = 67; goto _out; 
		_out68: cs = 68; goto _out; 
		_out69: cs = 69; goto _out; 
		_out70: cs = 70; goto _out; 
		_out71: cs = 71; goto _out; 
		_out72: cs = 72; goto _out; 
		_out73: cs = 73; goto _out; 
		_out74: cs = 74; goto _out; 
		_out75: cs = 75; goto _out; 
		_out76: cs = 76; goto _out; 
		_out77: cs = 77; goto _out; 
		_out78: cs = 78; goto _out; 
		_out79: cs = 79; goto _out; 
		_out80: cs = 80; goto _out; 
		_out81: cs = 81; goto _out; 
		_out82: cs = 82; goto _out; 
		_out83: cs = 83; goto _out; 
		_out84: cs = 84; goto _out; 
		_out85: cs = 85; goto _out; 
		_out86: cs = 86; goto _out; 
		_out87: cs = 87; goto _out; 
		_out88: cs = 88; goto _out; 
		_out89: cs = 89; goto _out; 
		_out90: cs = 90; goto _out; 
		_out91: cs = 91; goto _out; 
		_out92: cs = 92; goto _out; 
		_out93: cs = 93; goto _out; 
		_out94: cs = 94; goto _out; 
		_out95: cs = 95; goto _out; 
		_out96: cs = 96; goto _out; 
		_out97: cs = 97; goto _out; 
		_out98: cs = 98; goto _out; 
		_out99: cs = 99; goto _out; 
		_out100: cs = 100; goto _out; 
		_out101: cs = 101; goto _out; 
		_out102: cs = 102; goto _out; 
		_out103: cs = 103; goto _out; 
		_out104: cs = 104; goto _out; 
		_out105: cs = 105; goto _out; 
		_out106: cs = 106; goto _out; 
		_out107: cs = 107; goto _out; 
		_out108: cs = 108; goto _out; 
		_out109: cs = 109; goto _out; 
		_out110: cs = 110; goto _out; 
		_out111: cs = 111; goto _out; 
		_out112: cs = 112; goto _out; 
		_out113: cs = 113; goto _out; 
		_out114: cs = 114; goto _out; 
		_out115: cs = 115; goto _out; 
		_out116: cs = 116; goto _out; 
		_out117: cs = 117; goto _out; 
		_out118: cs = 118; goto _out; 
		_out119: cs = 119; goto _out; 
		_out120: cs = 120; goto _out; 
		_out121: cs = 121; goto _out; 
		_out122: cs = 122; goto _out; 
		_out123: cs = 123; goto _out; 
		_out124: cs = 124; goto _out; 
		_out125: cs = 125; goto _out; 
		_out126: cs = 126; goto _out; 
		_out127: cs = 127; goto _out; 
		_out128: cs = 128; goto _out; 
		_out129: cs = 129; goto _out; 
		_out130: cs = 130; goto _out; 
		_out131: cs = 131; goto _out; 
		_out132: cs = 132; goto _out; 
		_out133: cs = 133; goto _out; 
		_out134: cs = 134; goto _out; 
		_out135: cs = 135; goto _out; 
		_out136: cs = 136; goto _out; 
		_out137: cs = 137; goto _out; 
		_out138: cs = 138; goto _out; 
		_out139: cs = 139; goto _out; 
		_out140: cs = 140; goto _out; 
		_out141: cs = 141; goto _out; 
		_out142: cs = 142; goto _out; 
		_out143: cs = 143; goto _out; 
		_out144: cs = 144; goto _out; 
		_out145: cs = 145; goto _out; 
		_out146: cs = 146; goto _out; 
		_out147: cs = 147; goto _out; 
		_out148: cs = 148; goto _out; 
		_out149: cs = 149; goto _out; 
		_out150: cs = 150; goto _out; 
		_out151: cs = 151; goto _out; 
		_out152: cs = 152; goto _out; 
		_out153: cs = 153; goto _out; 
		_out154: cs = 154; goto _out; 
		_out155: cs = 155; goto _out; 
		_out156: cs = 156; goto _out; 
		_out157: cs = 157; goto _out; 
		_out158: cs = 158; goto _out; 
		_out159: cs = 159; goto _out; 
		_out160: cs = 160; goto _out; 
		_out161: cs = 161; goto _out; 
		_out162: cs = 162; goto _out; 
		_out163: cs = 163; goto _out; 
		_out164: cs = 164; goto _out; 
		_out165: cs = 165; goto _out; 
		_out166: cs = 166; goto _out; 
		_out167: cs = 167; goto _out; 
		_out168: cs = 168; goto _out; 
		_out169: cs = 169; goto _out; 
		_out170: cs = 170; goto _out; 
		_out171: cs = 171; goto _out; 
		_out172: cs = 172; goto _out; 
		_out173: cs = 173; goto _out; 
		_out174: cs = 174; goto _out; 
		_out175: cs = 175; goto _out; 
		_out176: cs = 176; goto _out; 
		_out177: cs = 177; goto _out; 
		_out178: cs = 178; goto _out; 
		_out179: cs = 179; goto _out; 
		_out180: cs = 180; goto _out; 
		_out181: cs = 181; goto _out; 
		_out182: cs = 182; goto _out; 
		_out183: cs = 183; goto _out; 
		_out184: cs = 184; goto _out; 
		_out185: cs = 185; goto _out; 
		_out186: cs = 186; goto _out; 
		_out187: cs = 187; goto _out; 
		_out188: cs = 188; goto _out; 
		_out189: cs = 189; goto _out; 
		_out190: cs = 190; goto _out; 
		_out191: cs = 191; goto _out; 
		_out192: cs = 192; goto _out; 
		_out193: cs = 193; goto _out; 
		_out194: cs = 194; goto _out; 
		_out195: cs = 195; goto _out; 
		_out196: cs = 196; goto _out; 
		_out197: cs = 197; goto _out; 
		_out349: cs = 349; goto _out; 
		_out198: cs = 198; goto _out; 
		_out199: cs = 199; goto _out; 
		_out200: cs = 200; goto _out; 
		_out201: cs = 201; goto _out; 
		_out202: cs = 202; goto _out; 
		_out203: cs = 203; goto _out; 
		_out204: cs = 204; goto _out; 
		_out205: cs = 205; goto _out; 
		_out206: cs = 206; goto _out; 
		_out207: cs = 207; goto _out; 
		_out208: cs = 208; goto _out; 
		_out209: cs = 209; goto _out; 
		_out210: cs = 210; goto _out; 
		_out211: cs = 211; goto _out; 
		_out212: cs = 212; goto _out; 
		_out213: cs = 213; goto _out; 
		_out214: cs = 214; goto _out; 
		_out215: cs = 215; goto _out; 
		_out216: cs = 216; goto _out; 
		_out217: cs = 217; goto _out; 
		_out218: cs = 218; goto _out; 
		_out219: cs = 219; goto _out; 
		_out220: cs = 220; goto _out; 
		_out221: cs = 221; goto _out; 
		_out222: cs = 222; goto _out; 
		_out223: cs = 223; goto _out; 
		_out224: cs = 224; goto _out; 
		_out225: cs = 225; goto _out; 
		_out226: cs = 226; goto _out; 
		_out227: cs = 227; goto _out; 
		_out228: cs = 228; goto _out; 
		_out229: cs = 229; goto _out; 
		_out230: cs = 230; goto _out; 
		_out231: cs = 231; goto _out; 
		_out232: cs = 232; goto _out; 
		_out233: cs = 233; goto _out; 
		_out234: cs = 234; goto _out; 
		_out235: cs = 235; goto _out; 
		_out236: cs = 236; goto _out; 
		_out237: cs = 237; goto _out; 
		_out238: cs = 238; goto _out; 
		_out239: cs = 239; goto _out; 
		_out240: cs = 240; goto _out; 
		_out241: cs = 241; goto _out; 
		_out242: cs = 242; goto _out; 
		_out243: cs = 243; goto _out; 
		_out244: cs = 244; goto _out; 
		_out245: cs = 245; goto _out; 
		_out246: cs = 246; goto _out; 
		_out247: cs = 247; goto _out; 
		_out248: cs = 248; goto _out; 
		_out249: cs = 249; goto _out; 
		_out250: cs = 250; goto _out; 
		_out251: cs = 251; goto _out; 
		_out252: cs = 252; goto _out; 
		_out253: cs = 253; goto _out; 
		_out254: cs = 254; goto _out; 
		_out255: cs = 255; goto _out; 
		_out256: cs = 256; goto _out; 
		_out257: cs = 257; goto _out; 
		_out258: cs = 258; goto _out; 
		_out259: cs = 259; goto _out; 
		_out260: cs = 260; goto _out; 
		_out261: cs = 261; goto _out; 
		_out262: cs = 262; goto _out; 
		_out263: cs = 263; goto _out; 
		_out264: cs = 264; goto _out; 
		_out265: cs = 265; goto _out; 
		_out266: cs = 266; goto _out; 
		_out267: cs = 267; goto _out; 
		_out268: cs = 268; goto _out; 
		_out269: cs = 269; goto _out; 
		_out270: cs = 270; goto _out; 
		_out271: cs = 271; goto _out; 
		_out272: cs = 272; goto _out; 
		_out273: cs = 273; goto _out; 
		_out274: cs = 274; goto _out; 
		_out275: cs = 275; goto _out; 
		_out276: cs = 276; goto _out; 
		_out277: cs = 277; goto _out; 
		_out278: cs = 278; goto _out; 
		_out279: cs = 279; goto _out; 
		_out280: cs = 280; goto _out; 
		_out281: cs = 281; goto _out; 
		_out282: cs = 282; goto _out; 
		_out283: cs = 283; goto _out; 
		_out284: cs = 284; goto _out; 
		_out285: cs = 285; goto _out; 
		_out286: cs = 286; goto _out; 
		_out287: cs = 287; goto _out; 
		_out288: cs = 288; goto _out; 
		_out289: cs = 289; goto _out; 
		_out290: cs = 290; goto _out; 
		_out291: cs = 291; goto _out; 
		_out292: cs = 292; goto _out; 
		_out293: cs = 293; goto _out; 
		_out294: cs = 294; goto _out; 
		_out295: cs = 295; goto _out; 
		_out296: cs = 296; goto _out; 
		_out297: cs = 297; goto _out; 
		_out298: cs = 298; goto _out; 
		_out299: cs = 299; goto _out; 
		_out300: cs = 300; goto _out; 
		_out301: cs = 301; goto _out; 
		_out302: cs = 302; goto _out; 
		_out303: cs = 303; goto _out; 
		_out304: cs = 304; goto _out; 
		_out305: cs = 305; goto _out; 
		_out306: cs = 306; goto _out; 
		_out307: cs = 307; goto _out; 
		_out308: cs = 308; goto _out; 
		_out309: cs = 309; goto _out; 
		_out310: cs = 310; goto _out; 
		_out311: cs = 311; goto _out; 
		_out312: cs = 312; goto _out; 
		_out313: cs = 313; goto _out; 
		_out314: cs = 314; goto _out; 
		_out315: cs = 315; goto _out; 
		_out316: cs = 316; goto _out; 
		_out317: cs = 317; goto _out; 
		_out318: cs = 318; goto _out; 
		_out319: cs = 319; goto _out; 
		_out320: cs = 320; goto _out; 
		_out321: cs = 321; goto _out; 
		_out322: cs = 322; goto _out; 
		_out323: cs = 323; goto _out; 
		_out324: cs = 324; goto _out; 
		_out325: cs = 325; goto _out; 
		_out326: cs = 326; goto _out; 
		_out327: cs = 327; goto _out; 
		_out328: cs = 328; goto _out; 
		_out329: cs = 329; goto _out; 
		_out330: cs = 330; goto _out; 
		_out331: cs = 331; goto _out; 
		_out332: cs = 332; goto _out; 
		_out333: cs = 333; goto _out; 
		_out334: cs = 334; goto _out; 
		_out335: cs = 335; goto _out; 
		_out336: cs = 336; goto _out; 
		_out337: cs = 337; goto _out; 
		_out338: cs = 338; goto _out; 
		_out339: cs = 339; goto _out; 
		_out340: cs = 340; goto _out; 
		_out341: cs = 341; goto _out; 
		_out342: cs = 342; goto _out; 
		_out343: cs = 343; goto _out; 
		_out344: cs = 344; goto _out; 
		_out345: cs = 345; goto _out; 
		_out346: cs = 346; goto _out; 
		_out347: cs = 347; goto _out; 
		_out: {}
	}
	
#line 298 "/home/xxxten/Xten_fiber_netlib/Xten/src/http/http11_parser.rl"

	
	assert(p <= pe && "Buffer overflow after parsing.");
	
	if (!http_parser_has_error(parser)) {
		parser->cs = cs;
	}
	
	parser->nread += p - (buffer + off);
	
	assert(parser->nread <= len && "nread longer than length");
	assert(parser->body_start <= len && "body starts after buffer end");
	assert(parser->mark < len && "mark is after buffer end");
	assert(parser->field_len <= len && "field has length longer than whole buffer");
	assert(parser->field_start < len && "field starts after buffer end");
	
	return(parser->nread);
}

int http_parser_finish(http_parser *parser)
{
	if (http_parser_has_error(parser) ) {
		return -1;
	} else if (http_parser_is_finished(parser) ) {
		return 1;
	} else {
		return 0;
	}
}

int http_parser_has_error(http_parser *parser) {
	return parser->cs == http_parser_error;
}

int http_parser_is_finished(http_parser *parser) {
	return parser->cs >= http_parser_first_final;
}
