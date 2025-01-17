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
// kgbest.cpp 指定行の選択クラス
// =============================================================================
#include <cstdio>
#include <iostream>
#include <kgbest.h>
#include <kgError.h>
#include <kgMethod.h>
#include <kgConfig.h>

using namespace std;
using namespace kglib;
using namespace kgmod;

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録)
// -----------------------------------------------------------------------------
const char * kgBest::_ipara[] = {"i",""};
const char * kgBest::_opara[] = {"o","u",""};

kgBest::kgBest(void)
{
	_name    = "kgbest";
	_version = "###VERSION###";

	_paralist = "R=,i=,o=,k=,u=,-r,to=,from=,fr=,size=,s=,-q";
	_paraflg = kgArgs::COMMON|kgArgs::IODIFF|kgArgs::NULL_KEY;

	#include <help/en/kgbestHelp.h>
	_titleL = _title;
	_docL   = _doc;
	#ifdef JPN_FORMAT
		#include <help/jp/kgbestHelp.h>
	#endif

}

// -----------------------------------------------------------------------------
// パラメータセット
// -----------------------------------------------------------------------------
void kgBest::setArgsMain(void)
{
	_iFile.read_header();

	// k= 項目引数のセット
	vector<kgstr_t> vs = _args.toStringVector("k=",false);


	// 範囲決定 & _range_maxのセット
	// R=の場合
	// _Rfield:R=の内容をセットする
	// _Rfield[0]に小さい値、_Rfield[0]に大きい値をセットする
	// from= [to=|size=]の場合
	// from=行No to=行No (Noは0開始) 
	// size=行数 
	_range_max=0;
	kgstr_t fr_str0 = _args.toString("from=",false);
	kgstr_t fr_str1 = _args.toString("fr=",false);
	if(!fr_str0.empty()&&!fr_str1.empty()){
		throw kgError("fr= cannot be specified with from=");
	}
	kgstr_t fr_str= fr_str0;
	if(!fr_str1.empty()){ fr_str= fr_str1;}
	
	kgstr_t to_str = _args.toString("to=",false);
	kgstr_t sz_str = _args.toString("size=",false);
	vector< vector<kgstr_t> > vvs = _args.toStringVecVec("R=","_:",2,false);

	// エラーチェック
	if( !vvs[0].empty() && ( !fr_str.empty()|| !to_str.empty()||!sz_str.empty())){
		throw kgError("R= cannot be specified with from=,fr=,to=,size=");
	}
	if(!to_str.empty()&&!sz_str.empty() ) {
		throw kgError("to= cannot be specified with size=");
	}

	// _Rfieldにセット
	if(!vvs[0].empty()){
		vector<kgstr_t>::size_type size = vvs[0].size();
		vector<size_t> vi(size);
		for(int i=0;i<2;i++){ _Rfield.push_back(vi); }
		for(vector<kgstr_t>::size_type i=0;i<size;i++){
			size_t f1,f2;
			const char *p1 = vvs[0][i].c_str();
			if			(!strcmp(p1,"MIN"))	{ f1 = 1; }
			else if (!strcmp(p1,"MAX"))	{ f1 = KG_SIZE_MAX;}
			else												{ f1 = aToSizeT(p1);}
			if(vvs[1][i].empty()){ f2 = f1;}
			else {
				const char *p2 = vvs[1][i].c_str();
				if			(!strcmp(p2,"MIN"))	{ f2 = 1; }
				else if (!strcmp(p2,"MAX"))	{ f2 = KG_SIZE_MAX;}
				else												{ f2 = aToSizeT(p2);}
			}
			if(f2>f1){ 
				if(f1!=0){ f1=f1-1;}
				_Rfield[0][i]=f1;		
				_Rfield[1][i]=f2;		
				if(_range_max<f2){ _range_max=f2;}
			}
			else{
				if(f2!=0){ f2=f2-1;}
				_Rfield[0][i]=f2;		
				_Rfield[1][i]=f1;
				if(_range_max<f1){ _range_max=f1;}
			}
		}
	}
	else{
		vector<size_t> vi(1);
		for(int i=0;i<2;i++){ _Rfield.push_back(vi); }
		size_t f1,f2;
		const char *p1 = fr_str.c_str();
		if          (*p1=='\0')					{ f1 = 0; }
		else if			(!strcmp(p1,"MIN"))	{ f1 = 0; }
		else														{ f1 = aToSizeT(p1);}
		if(!to_str.empty()){
			const char *p2 = to_str.c_str();
			if		(!strcmp(p2,"MAX"))	{ f2 = KG_SIZE_MAX; }
			else{
				size_t ft = aToSizeT(p2);
				if(f1 < ft ){ f2 = ft+1; }
				else {
					f2 = f1 +1;
					f1 = ft;
				}
			}
		}
		else{
			if(sz_str.empty()) { f2 = f1+1;}
			else							 { 
				f2 = aToSizeT(sz_str.c_str())+f1;
			}
		}
		_Rfield[0][0]=f1;		
		_Rfield[1][0]=f2;		
		if(_range_max<f2){ _range_max=f2;}
	}

	vector<kgstr_t> vss = _args.toStringVector("s=",false);
	bool seqflg = _args.toBool("-q");
	if(_nfn_i) { seqflg = true; }

	if(!seqflg&&vss.empty()){
		throw kgError("parameter s= is mandatory without -q,-nfn");
	}


	if(!seqflg && (!vs.empty()||!vss.empty())){ 
		vector<kgstr_t> vsk	= vs;
		vsk.insert(vsk.end(),vss.begin(),vss.end());
		sortingRun(&_iFile,vsk);
	}
	_kField.set(vs,  &_iFile, _fldByNum);

}

void kgBest::setArgs(void)
{
	// パラメータチェック
	_args.paramcheck(_paralist,_paraflg);

	// 入出力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);
	kgstr_t ufile = _args.toString("u=",false);
	if(ufile.empty()){ _elsefile=false; }
	else {
		_elsefile=true;
		_uFile.open(ufile,_env,_nfn_o);
	}
	// -r 条件反転: _outputは通常出力ならtrue,
	_output = !_args.toBool("-r");
	setArgsMain();
}

// -----------------------------------------------------------------------------
// パラメータセット＆入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgBest::setArgs(int inum,int *i_p,int onum ,int *o_p)
{
	int iopencnt = 0;
	int oopencnt = 0;
	try {
		// パラメータチェック
		_args.paramcheck(_paralist,_paraflg);

		// -r 条件反転: _outputは通常出力ならtrue,
		_output = !_args.toBool("-r");

		if(inum>1 || onum>2){ throw kgError("no match IO");}

		// 入出力ファイルオープン
		if(inum==1 && *i_p>0){ _iFile.popen(*i_p, _env,_nfn_i); }
		else     { _iFile.open(_args.toString("i=",true), _env,_nfn_i); }
		iopencnt++;


		// 出力チェック
		kgstr_t okwd = "o=";
		kgstr_t ukwd = "u=";
		kgstr_t ofile0	= _args.toString(okwd,false);
		kgstr_t ufile0  = _args.toString(ukwd,false);
		int o_no = -1;
		int u_no = -1;
		if(onum>0){ o_no = *o_p;}
		if(onum>1){ u_no = *(o_p+1);}
		if(o_no==-1 && ofile0.empty()){
			if(u_no!=-1 || !ufile0.empty()){
				_output = !_output;
				kgstr_t swptmp;
				swptmp = ukwd ; ukwd = okwd;  okwd = swptmp;
				int swptmpi;
				swptmpi = u_no ; u_no = o_no ; o_no = swptmpi;
			}
		}


		if(o_no>0){ _oFile.popen(o_no, _env,_nfn_o);}
		else     { _oFile.open(_args.toString(okwd,true), _env,_nfn_o);}
		oopencnt++;

		kgstr_t ufile = _args.toString(ukwd,false);

		if(u_no>0){ 
			_uFile.popen(*(o_p+1), _env,_nfn_o); 
			_elsefile=true;
			oopencnt++;
		}
		else if(ufile.empty()){
			_elsefile=false;
		}
		else{
			_uFile.open(ufile,_env,_nfn_o);
			_elsefile=true;
		}
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
// _Rfieldにセットされた範囲内にあるならtrue、無いならfalseを返す
// -----------------------------------------------------------------------------
bool kgBest::IsInRange(size_t val) 
{ 
	vector<size_t>::size_type size = _Rfield[0].size(); 
	for(vector<int>::size_type i=0;i<size;i++){
		if(_Rfield[0][i]<=val && val<_Rfield[1][i]){ return true;}
	}
	return false;
}
// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgBest::runMain(void)
{

	// 入力ファイルにkey項目番号をセットする．
	_iFile.setKey(_kField.getNum());

	// 項目名出力
	_oFile.writeFldName(_iFile);
	if(_elsefile){ _uFile.writeFldName(_iFile); }

	// データセレクト＆出力
	// keybreakしたらcnt初期化
	// 通常処理；指定範囲内なら出力（-r:のしていが有ると反転）
	//         u=の指定が有る場合範囲外を別に出力
	size_t cnt=0;
	while(_iFile.read()!=EOF){
		if( _iFile.keybreak() ){
			if((_iFile.status() & kgCSV::End )) break;
			cnt=0;
		}
		// keyの指定が無く、max行を超えて、通常出力、不一致出力がなければ終了
		if(_kField.size()==0 ){
			if(cnt>=_range_max&& _output==true&&_elsefile==false){break;}
		}	
		// 通常行処理 
		if( IsInRange(cnt) == _output){
			_oFile.writeFld(_iFile.fldSize(),_iFile.getNewFld());
		}
		else if(_elsefile==true){
			_uFile.writeFld(_iFile.fldSize(),_iFile.getNewFld());
		}
		cnt++;
	}
	//ソートスレッドを終了させて、終了確認=DEBUG
  //for(size_t i=0 ;i<_th_st.size();i++){ pthread_cancel(_th_st[i]->native_handle());	}
  //for(size_t i=0 ;i<_th_st.size();i++){ pthread_join(_th_st[i]->native_handle(),NULL);}
	//ASSERT keynull_CHECK
	if(_assertNullKEY) { _existNullKEY = _iFile.keynull(); }
	// 終了処理
	th_cancel();
	_iFile.close();
	_oFile.close();
	if(_elsefile){ _uFile.close();}

	return 0;

}

// -----------------------------------------------------------------------------
// 実行 
// -----------------------------------------------------------------------------
int kgBest::run(void) 
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
    ((kgBest*)arg)->runErrEnd();
}


int kgBest::run(int inum,int *i_p,int onum, int* o_p,string &msg)
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
