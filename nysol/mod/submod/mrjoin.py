# -*- coding: utf-8 -*-
import nysol._nysolshell_core as n_core
from nysol.mod.nysollib.nysolmodcore import NysolMOD_CORE
from nysol.mod.nysollib import nysolutil as nutil

class Nysol_Mrjoin(NysolMOD_CORE):
	kwd = n_core.getparalist("mrjoin")
	def __init__(self,*args, **kw_args) :
		super(Nysol_Mrjoin,self).__init__("mrjoin",nutil.args2dict(args,kw_args,Nysol_Mrjoin.kwd))
