/* ////////// LICENSE INFO ////////////////////

 * Copyright (C) 2013 by NYSOL CORPORATION
 *
 * Unless you have received this program directly from NYSOL pursuant
 * to the terms of a commercial license agreement with NYSOL, then
 * this program is licensed to you under the terms of the GNU Affero General
 * Public License (AGPL) as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF 
 * NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Please refer to the AGPL (http://www.gnu.org/licenses/agpl-3.0.txt)
 * for more details.

 ////////// LICENSE INFO ////////////////////*/
// =============================================================================
// kgnormalize.cpp 数値基準化クラス
// =============================================================================
#include <cstdio>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <kgnormalize.h>
#include <kgError.h>
#include <kgMethod.h>
#include <kgConfig.h>

using namespace std;
using namespace kglib;
using namespace kgmod;

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録)
// -----------------------------------------------------------------------------
const char * kgNormalize::_ipara[] = {"i",""};
const char * kgNormalize::_opara[] = {"o",""};

kgNormalize::kgNormalize(void)
{
	_name    = "kgnormalize";
	_version = "###VERSION###";

	_paralist = "k=,f=,c=,i=,o=,bufcount=,-q";
	_paraflg = kgArgs::ALLPARAM;

	#include <help/en/kgnormalizeHelp.h>
	_titleL = _title;
	_docL   = _doc;
	#ifdef JPN_FORMAT
		#include <help/jp/kgnormalizeHelp.h>
	#endif

}

namespace kgnormalize_ 
{
	// ---------------------------------------------------------------------------
	// zscore
	// ---------------------------------------------------------------------------
	void zscore(kgCSVblk& csv, kgArgFld& fld, kgCSVout& csvOut,
			bool a_Nin ,bool a_Nout ,bool* e_Nin ,bool* e_Nout )
	{
		// 集計用変数領域確保＆初期化
		int fldSize=fld.size();

		// 合計,件数,偏差平方和を求める
		vector<double> sum(fldSize,0);
		vector<double> cnt(fldSize,0);
		vector<double> dv2(fldSize,0);
		vector<double> sx(fldSize,0);

		while(csv.blkread() != EOF){
			for(int i=0; i<fldSize; i++){
				char* str=csv.getBlkVal(fld.num(i));
				// データがnullの場合は何もしない
				if(*str=='\0'){
					if(a_Nin) { *e_Nin  = true;}
					continue;
				}
				double dx = atof(str);
				sum.at(i)+=dx;
				dx -= sx.at(i);
				sx.at(i) += dx / (cnt.at(i)+1); 
				dv2.at(i) += cnt.at(i) * dx * dx / (cnt.at(i)+1); 
				cnt.at(i) += 1;
			}
		}
		// 平均と標準偏差(不偏)
		vector<double> avg(fldSize,0);
		vector<double> std(fldSize,0);
		for(int i=0; i<fldSize; i++){
			if(cnt.at(i)>1){
				avg.at(i)=sum.at(i)/cnt.at(i);
				std.at(i)=sqrt(dv2.at(i)/(cnt.at(i)-1));
			}
		}
		// 出力
		csv.seekBlkTop();
		while(csv.blkread() != EOF){
			csvOut.writeFld(csv.fldSize(), csv.getBlkFld(), false);
			for(int i=0; i<fldSize; i++){
				bool eol=false;
				if(i==fldSize-1) eol=true;
				char* str=csv.getBlkVal(fld.num(i));
				if(*str=='\0' || cnt.at(i)<2 || std.at(i)==0){
					if(a_Nout) { *e_Nout  = true;}
					csvOut.writeStr("",eol);
				}else{
					double dx = atof(str);
					csvOut.writeDbl( (dx-avg.at(i))/std.at(i), eol );
				}
			}
		}
	}
	void zscorep(kgCSVblk& csv, kgArgFld& fld, kgCSVout& csvOut,
			bool a_Nin ,bool a_Nout ,bool* e_Nin ,bool* e_Nout )
	{
		// 集計用変数領域確保＆初期化
		int fldSize=fld.size();

		// 合計,件数,偏差平方和を求める
		vector<double> sum(fldSize,0);
		vector<double> cnt(fldSize,0);
		vector<double> dv2(fldSize,0);
		vector<double> sx(fldSize,0);

		while(csv.blkread() != EOF){
			for(int i=0; i<fldSize; i++){
				char* str=csv.getBlkVal(fld.num(i));
				// データがnullの場合は何もしない
				if(*str=='\0'){
					if(a_Nin) { *e_Nin  = true;}
					continue;
				}
				double dx = atof(str);
				sum.at(i)+=dx;
				dx -= sx.at(i);
				sx.at(i) += dx / (cnt.at(i)+1); 
				dv2.at(i) += cnt.at(i) * dx * dx / (cnt.at(i)+1); 
				cnt.at(i) += 1;
			}
		}
		// 平均と標準偏差(不偏)
		vector<double> avg(fldSize,0);
		vector<double> std(fldSize,0);
		for(int i=0; i<fldSize; i++){
			if(cnt.at(i)>1){
				avg.at(i)=sum.at(i)/cnt.at(i);
				std.at(i)=sqrt(dv2.at(i)/cnt.at(i));
			}
		}
		// 出力
		csv.seekBlkTop();
		while(csv.blkread() != EOF){
			csvOut.writeFld(csv.fldSize(), csv.getBlkFld(), false);
			for(int i=0; i<fldSize; i++){
				bool eol=false;
				if(i==fldSize-1) eol=true;
				char* str=csv.getBlkVal(fld.num(i));
				if(*str=='\0' || cnt.at(i)<2 || std.at(i)==0){
					if(a_Nout) { *e_Nout  = true;}
					csvOut.writeStr("",eol);
				}else{
					double dx = atof(str);
					csvOut.writeDbl( (dx-avg.at(i))/std.at(i), eol );
				}
			}
		}
	}

	// ---------------------------------------------------------------------------
	// Zscore
	// ---------------------------------------------------------------------------
	void Zscore(kgCSVblk& csv, kgArgFld& fld, kgCSVout& csvOut,
			bool a_Nin ,bool a_Nout ,bool* e_Nin ,bool* e_Nout )
	{
		// 集計用変数領域確保＆初期化
		int fldSize=fld.size();

		// 合計,件数,偏差平方和を求める
		vector<double> sum(fldSize,0);
		vector<double> cnt(fldSize,0);
		vector<double> dv2(fldSize,0);
		vector<double> sx(fldSize,0);

		while(csv.blkread() != EOF){
			for(int i=0; i<fldSize; i++){
				char* str=csv.getBlkVal(fld.num(i));
				// データがnullの場合は何もしない
				if(*str=='\0'){
					if(a_Nin) { *e_Nin  = true;}
					continue;
				}
				double dx = atof(str);
				sum.at(i)+=dx;
				dx -= sx.at(i);
				sx.at(i) += dx / (cnt.at(i)+1); 
				dv2.at(i) += cnt.at(i) * dx * dx / (cnt.at(i)+1); 
				cnt.at(i) += 1;
			}
		}
		// 平均と標準偏差(不偏)
		vector<double> avg(fldSize,0);
		vector<double> std(fldSize,0);
		for(int i=0; i<fldSize; i++){
			if(cnt.at(i)>1){
				avg.at(i)=sum.at(i)/cnt.at(i);
				std.at(i)=sqrt(dv2.at(i)/(cnt.at(i)-1));
			}
		}
		// 出力
		csv.seekBlkTop();
		while(csv.blkread() != EOF){
			csvOut.writeFld(csv.fldSize(), csv.getBlkFld(), false);
			for(int i=0; i<fldSize; i++){
				bool eol=false;
				if(i==fldSize-1) eol=true;
				char* str=csv.getBlkVal(fld.num(i));
				if(*str=='\0' || cnt.at(i)<2 || std.at(i)==0){
					if(a_Nout) { *e_Nout  = true;}
					csvOut.writeStr("",eol);
				}else{
					double dx = atof(str);
					csvOut.writeDbl( 50.0+10.0*(dx-avg.at(i))/std.at(i), eol );
				}
			}
		}
	}
	void Zscorep(kgCSVblk& csv, kgArgFld& fld, kgCSVout& csvOut,
			bool a_Nin ,bool a_Nout ,bool* e_Nin ,bool* e_Nout )
	{
		// 集計用変数領域確保＆初期化
		int fldSize=fld.size();

		// 合計,件数,偏差平方和を求める
		vector<double> sum(fldSize,0);
		vector<double> cnt(fldSize,0);
		vector<double> dv2(fldSize,0);
		vector<double> sx(fldSize,0);

		while(csv.blkread() != EOF){
			for(int i=0; i<fldSize; i++){
				char* str=csv.getBlkVal(fld.num(i));
				// データがnullの場合は何もしない
				if(*str=='\0'){
					if(a_Nin) { *e_Nin  = true;}
					continue;
				}
				double dx = atof(str);
				sum.at(i)+=dx;
				dx -= sx.at(i);
				sx.at(i) += dx / (cnt.at(i)+1); 
				dv2.at(i) += cnt.at(i) * dx * dx / (cnt.at(i)+1); 
				cnt.at(i) += 1;
			}
		}
		// 平均と標準偏差(不偏)
		vector<double> avg(fldSize,0);
		vector<double> std(fldSize,0);
		for(int i=0; i<fldSize; i++){
			if(cnt.at(i)>1){
				avg.at(i)=sum.at(i)/cnt.at(i);
				std.at(i)=sqrt(dv2.at(i)/(cnt.at(i)));
			}
		}
		// 出力
		csv.seekBlkTop();
		while(csv.blkread() != EOF){
			csvOut.writeFld(csv.fldSize(), csv.getBlkFld(), false);
			for(int i=0; i<fldSize; i++){
				bool eol=false;
				if(i==fldSize-1) eol=true;
				char* str=csv.getBlkVal(fld.num(i));
				if(*str=='\0' || cnt.at(i)<2 || std.at(i)==0){
					if(a_Nout) { *e_Nout  = true;}
					csvOut.writeStr("",eol);
				}else{
					double dx = atof(str);
					csvOut.writeDbl( 50.0+10.0*(dx-avg.at(i))/std.at(i), eol );
				}
			}
		}
	}

	// ---------------------------------------------------------------------------
	// range
	// ---------------------------------------------------------------------------
	void range(kgCSVblk& csv, kgArgFld& fld, kgCSVout& csvOut,
			bool a_Nin ,bool a_Nout ,bool* e_Nin ,bool* e_Nout )
	{
		// 集計用変数領域確保＆初期化
		int fldSize=fld.size();

		// 合計,件数,偏差平方和を求める
		vector<double> minimum(fldSize, DBL_MAX);
		vector<double> maximum(fldSize,-DBL_MAX);
		vector<double> cnt(fldSize,0);
		vector<double> sx(fldSize,0);

		while(csv.blkread() != EOF){
			for(int i=0; i<fldSize; i++){
				char* str=csv.getBlkVal(fld.num(i));
				// データがnullの場合は何もしない
				if(*str=='\0'){
					if(a_Nin) { *e_Nin  = true;}
					continue;
				}
				double dx = atof(str);
				if(minimum.at(i)>dx) minimum.at(i)=dx;
				if(maximum.at(i)<dx) maximum.at(i)=dx;
				cnt.at(i) += 1;
			}
		}
		// nullフラグ，範囲
		vector<bool>   nul(fldSize,false);
		vector<double> rng(fldSize,false);
		for(int i=0; i<fldSize; i++){
			if(cnt.at(i)>0){
				rng.at(i)=maximum.at(i)-minimum.at(i);
				if(rng.at(i)==0){
					nul.at(i)=true;
				}
			}else{
				nul.at(i)=true;
			}
		}

		// 出力
		csv.seekBlkTop();
		while(csv.blkread() != EOF){
			csvOut.writeFld(csv.fldSize(), csv.getBlkFld(), false);
			for(int i=0; i<fldSize; i++){
				bool eol=false;
				if(i==fldSize-1){ eol=true; }
				char* str=csv.getBlkVal(fld.num(i));
				if(*str=='\0' || nul.at(i) || rng.at(i)==0){
					if(a_Nout) { *e_Nout  = true;}
					csvOut.writeStr("",eol);
				}else{
					double dx = atof(str);
					csvOut.writeDbl( (dx-minimum.at(i))/rng.at(i), eol );
				}
			}
		}
	}
}
// -----------------------------------------------------------------------------
// パラメータセット＆入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgNormalize::setArgsMain(void)
{
	kgstr_t s=_args.toString("bufcount=",false);
	int bcnt = 10;
	if(!s.empty()){ 
		bcnt = atoi(s.c_str());
		if(bcnt<=0){ bcnt=1;}
	}
	_iFile.setbufsize(bcnt);
	_iFile.read_header();
  _oFile.setPrecision(_precision);

	// f= 項目引数のセット
	vector< vector<kgstr_t> > vvs = _args.toStringVecVec("f=",':',2,true);

	// k= 項目引数のセット
	vector<kgstr_t> vs = _args.toStringVector("k=",false);

	// c= 計算方法のセット
	_c_type = _args.toString("c=", true);
     	 if(_c_type=="z"     ){_function=&kgnormalize_::zscore;}
  else if(_c_type=="Z"     ){_function=&kgnormalize_::Zscore;}
  else if(_c_type=="zp"     ){_function=&kgnormalize_::zscorep;}
  else if(_c_type=="Zp"     ){_function=&kgnormalize_::Zscorep;}
	else if(_c_type=="range" ){_function=&kgnormalize_::range; }
	else {	throw kgError("c= takes one of `z',`Z' and `range'");	}

	bool seqflg = _args.toBool("-q");
	if(_nfn_i) { seqflg = true; }

	if(!seqflg && !vs.empty()){ sortingRun(&_iFile,vs);}

	_kField.set(vs,  &_iFile,_fldByNum);
	_fField.set(vvs, &_iFile,_fldByNum);

}

// -----------------------------------------------------------------------------
// パラメータセット＆入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgNormalize::setArgs(void)
{
	// パラメータチェック
	_args.paramcheck(_paralist,_paraflg);

	// 入出力ファイルオープン&バッファセット
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);

}
// -----------------------------------------------------------------------------
// パラメータセット＆入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgNormalize::setArgs(int inum,int *i_p,int onum ,int *o_p)
{
	int iopencnt = 0;
	int oopencnt = 0;
	try{
		_args.paramcheck(_paralist,_paraflg);

		if(inum>1 || onum>1){ throw kgError("no match IO");}

		if(inum==1 && *i_p>0){ _iFile.popen(*i_p, _env,_nfn_i); }
		else     { _iFile.open(_args.toString("i=",true), _env,_nfn_i); }
		iopencnt++;

		if(onum==1 && *o_p>0){ _oFile.popen(*o_p, _env,_nfn_o); }
		else     { _oFile.open(_args.toString("o=",true), _env,_nfn_o);}
		oopencnt++;

		setArgsMain();

	}catch(...){
		for(int i=iopencnt; i<inum ;i++){
			if(*(i_p+i)>0){ ::close(*(i_p+i)); }
		}
		for(int i=oopencnt; i<onum ;i++){
			if(*(o_p+i)>0){ ::close(*(o_p+i)); }
		}
		throw;
	}
		

}
// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgNormalize::runMain(void)
{

	// 入力ファイルにkey項目番号をセットする．
	_iFile.setKey(_kField.getNum());

	// 項目名の出力
  _oFile.writeFldName(_iFile,_fField, true);

	//キー単位で読み込み一時ファイルに出力
	while(_iFile.blkset()!=EOF){
		// 計算の実行本体
		_function(_iFile, _fField, _oFile,_assertNullIN,_assertNullOUT,&_existNullIN,&_existNullOUT);
	}

	// 終了処理
	th_cancel();
	_iFile.close();
	_oFile.close();

	return 0;

}

// -----------------------------------------------------------------------------
// 実行 
// -----------------------------------------------------------------------------
int kgNormalize::run(void) 
{
	try {

		setArgs();
		int sts = runMain();
		successEnd();
		return sts;

	}catch(kgOPipeBreakError& err){

		runErrEnd();
		successEnd();
		return 0;

	}catch(kgError& err){

		runErrEnd();
		errorEnd(err);
	}catch (const exception& e) {

		runErrEnd();
		kgError err(e.what());
		errorEnd(err);
	}catch(char * er){

		runErrEnd();
		kgError err(er);
		errorEnd(err);
	}catch(...){

		runErrEnd();
		kgError err("unknown error" );
		errorEnd(err);
	}
	return 1;

}

///* thraad cancel action
static void cleanup_handler(void *arg)
{
    ((kgNormalize*)arg)->runErrEnd();
}

int kgNormalize::run(int inum,int *i_p,int onum, int* o_p,string &msg)
{
	int sts=1;
	pthread_cleanup_push(&cleanup_handler, this);	

	try {

		setArgs(inum, i_p, onum,o_p);
		sts = runMain();
		msg.append(successEndMsg());

	}catch(kgOPipeBreakError& err){

		runErrEnd();
		msg.append(successEndMsg());
		sts = 0;

	}catch(kgError& err){

		runErrEnd();
		msg.append(errorEndMsg(err));

	}catch (const exception& e) {

		runErrEnd();
		kgError err(e.what());
		msg.append(errorEndMsg(err));

	}catch(char * er){

		runErrEnd();
		kgError err(er);
		msg.append(errorEndMsg(err));

	}
	KG_ABI_CATCH
	catch(...){

		runErrEnd();
		kgError err("unknown error" );
		msg.append(errorEndMsg(err));

	}
	pthread_cleanup_pop(0);
	return sts;
}

