if COMPILE_PHAPI
lib_LIBRARIES = libamr.a
endif

if COMPILE_PHAPI
bin_PROGRAMS = coder decoder
coder_SOURCES = coder.c
decoder_SOURCES = decoder.c
endif

AM_CFLAGS=@CFLAGS@

TABFILES=bitno.tab c2_11pf.tab c2_9pf.tab corrwght.tab d_homing.tab gains.tab gray.tab grid.tab inter_36.tab inv_sqrt.tab lag_wind.tab log2.tab lsp_lsf.tab lsp.tab ph_disp.tab pow2.tab qgain475.tab q_plsf_3.tab q_plsf_5.tab qua_gain.tab sqrt_l.tab window.tab

EXTRA_DIST = agc.h amr.h a_refl.h autocorr.h az_lsp.h basic_op.h b_cn_cod.h bgnscd.h bits2prm.h c1035pf.h \
  c2_11pf.h c2_9pf.h c3_14pf.h c4_17pf.h c8_31pf.h calc_cor.h calc_en.h cbsearch.h c_g_aver.h \
 cl_ltp.h cnst.h \
 cnst_vad.h cod_amr.h convolve.h copy.h cor_h.h count.h \
 d1035pf.h d2_11pf.h d2_9pf.h d3_14pf.h d4_17pf.h d8_31pf.h dec_amr.h \
 dec_gain.h dec_lag3.h dec_lag6.h d_gain_c.h d_gain_p.h d_homing.h d_plsf.h \
 dtx_dec.h dtx_enc.h ec_gains.h e_homing.h enc_lag3.h enc_lag6.h ex_ctrl.h  \
 frame.h g_adapt.h gain_q.h g_code.h gc_pred.h gmed_n.h g_pitch.h hp_max.h inter_36.h int_lpc.h int_lsf.h inv_sqrt.h lag_wind.h \
 levinson.h log2.h lpc.h lsfwt.h lsp_avg.h lsp_az.h lsp.h lsp_lsf.h mac_32.h mode.h n_proc.h \
 ol_ltp.h oper_32b.h ph_disp.h pitch_fr.h pitch_ol.h p_ol_wgh.h post_pro.h pow2.h pre_big.h pred_lt.h preemph.h \
 pre_proc.h prm2bits.h pstfilt.h qgain475.h qgain795.h q_gain_c.h q_gain_p.h q_plsf.h \
 qua_gain.h reorder.h residu.h s10_8pf.h set_sign.h set_zero.h sid_sync.h sp_dec.h sp_enc.h spreproc.h spstproc.h \
 sqrt_l.h strfunc.h syn_filt.h ton_stab.h typedef.h typedefs.h vad1.h vad2.h vad.h vadname.h weight_a.h $(TABFILES)




libamr_a_SOURCES =  agc.c autocorr.c az_lsp.c bits2prm.c \
       cl_ltp.c convolve.c c1035pf.c d_plsf.c d_plsf_5.c \
       d_gain_c.c d_gain_p.c dec_lag6.c d1035pf.c cor_h.c \
       enc_lag3.c enc_lag6.c g_code.c g_pitch.c int_lpc.c \
       inter_36.c inv_sqrt.c \
       lag_wind.c levinson.c lsp_az.c lsp_lsf.c ol_ltp.c \
       pitch_fr.c pitch_ol.c pow2.c pre_big.c pre_proc.c pred_lt.c preemph.c \
       prm2bits.c \
       pstfilt.c q_gain_c.c q_gain_p.c q_plsf.c q_plsf_5.c lsfwt.c reorder.c \
       residu.c lsp.c lpc.c ec_gains.c spreproc.c syn_filt.c \
       weight_a.c qua_gain.c gc_pred.c q_plsf_3.c post_pro.c \
       dec_lag3.c dec_gain.c d_plsf_3.c d4_17pf.c c4_17pf.c d3_14pf.c \
       c3_14pf.c \
       d2_11pf.c c2_11pf.c d2_9pf.c c2_9pf.c cbsearch.c spstproc.c gain_q.c \
       cod_amr.c dec_amr.c sp_enc.c sp_dec.c ph_disp.c \
       g_adapt.c calc_en.c qgain795.c qgain475.c sqrt_l.c set_sign.c s10_8pf.c \
       bgnscd.c gmed_n.c \
       mac_32.c ex_ctrl.c c_g_aver.c lsp_avg.c int_lsf.c c8_31pf.c d8_31pf.c \
       p_ol_wgh.c ton_stab.c vad1.c dtx_enc.c dtx_dec.c a_refl.c \
       b_cn_cod.c calc_cor.c hp_max.c vadname.c \
       vad2.c r_fft.c lflg_upd.c \
       e_homing.c d_homing.c \
	basicop2.c count.c oper_32b.c copy.c log2.c set_zero.c \
           strfunc.c n_proc.c sid_sync.c


coder_LDADD =  -L$(top_builddir)/phapi/amr -lamr  
decoder_LDADD =  -L$(top_builddir)/phapi/amr -lamr  