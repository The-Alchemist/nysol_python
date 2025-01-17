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
// kgnrjoin.h 参照ファイル項目の数値範囲マッチング自然結合クラス
// =============================================================================
#pragma once
#include <kgmodincludesort.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>

using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace

class kgNrjoin:public kgModIncludeSort 
{
	// 引数
	kgArgFld _kField;  // k=
	kgArgFld _KField;  // K=
	kgArgFld _fField;  // f=
	kgArgFld _RField;  // R=
	kgArgFld _rField;  // r=
	kgCSVkey _iFile;   // i=
	kgCSVkey _mFile;   // m=
	kgCSVout _oFile;   // o=
	bool     _i_outer; // -n
	bool     _m_outer; // -N

	//	比較方法(初期値false) 
	//	true:数字ソート false:文字ソート
	bool _cmp_type;

	// 範囲種類(true:equalも含む false:equalも含まない)
	// 初期値_range1:true,_range1:false 
	bool _range1_type,_range2_type;

	// 引数セット
  void setArgs(void);
	void setArgs(int inum,int *i,int onum, int* o);
	void setArgsMain(void);	

	int runMain(void);

	// 範囲マッチ
	void range_match(void);

	// CSV項目出力
	void writeFld(const vector<int>* flg,char** fld1,char** fld2 , int addsize);


public:
	static const char * _ipara[];
	static const char * _opara[];

	// コンストラクタ
	kgNrjoin(void);
	~kgNrjoin(void){}

	// コマンド固有の公開メソッド
	size_t iRecNo(void) const { return _iFile.recNo(); }
	size_t mRecNo(void) const { return _mFile.recNo(); }
	size_t oRecNo(void) const { return _oFile.recNo(); }
	//実行メソッド
	int run(void);
	int run(int inum,int *i_p,int onum, int* o_p ,string & str);
	void runErrEnd(void){
		th_cancel();
		_iFile.close();
		_mFile.close();
		_oFile.forceclose();
	}


};

}
