/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "rap.h"

bool_t
xdr_RAP_BYTE_TYPE (XDR *xdrs, RAP_BYTE_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_u_char (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_USHORT_TYPE (XDR *xdrs, RAP_USHORT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_u_short (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_SHORT_TYPE (XDR *xdrs, RAP_SHORT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_short (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_BOOL_TYPE (XDR *xdrs, RAP_BOOL_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_FLOAT_TYPE (XDR *xdrs, RAP_FLOAT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_float (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_LONG_TYPE (XDR *xdrs, RAP_LONG_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_long (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_STATUS_TYPE (XDR *xdrs, RAP_STATUS_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_long (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_STRING_TYPE (XDR *xdrs, RAP_STRING_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, objp, 80))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_FILE_DESCRIPTOR (XDR *xdrs, FILE_DESCRIPTOR *objp)
{
	register int32_t *buf;

	 if (!xdr_long (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_LONG_TYPE (XDR *xdrs, RAPVAR_LONG_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_long (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_DOMAIN_TYPE (XDR *xdrs, RAPVAR_DOMAIN_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_USHORT_TYPE (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_NAME_TYPE (XDR *xdrs, RAPVAR_NAME_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_STRING_TYPE (xdrs, &objp->name))
		 return FALSE;
	 if (!xdr_RAP_STRING_TYPE (xdrs, &objp->type))
		 return FALSE;
	 if (!xdr_RAPVAR_DOMAIN_TYPE (xdrs, &objp->domain))
		 return FALSE;
	 if (!xdr_RAP_SHORT_TYPE (xdrs, &objp->number1))
		 return FALSE;
	 if (!xdr_RAP_SHORT_TYPE (xdrs, &objp->number2))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAP_HEAD_TYPE (XDR *xdrs, RAP_HEAD_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_long (xdrs, &objp->userdef))
		 return FALSE;
	 if (!xdr_long (xdrs, &objp->data1))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_NUM_TYPE (XDR *xdrs, RAPVAR_NUM_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_FLOAT_TYPE (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_STRING_TYPE (XDR *xdrs, RAPVAR_STRING_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_STRING_TYPE (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_BOOL_TYPE (XDR *xdrs, RAPVAR_BOOL_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_FLOAT_TYPE (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_NUMBER_TYPE (XDR *xdrs, RAPVAR_NUMBER_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_SHORT_TYPE (XDR *xdrs, RAPVAR_SHORT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_SHORT_TYPE (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_ARRAY_DIM_TYPE (XDR *xdrs, RAPVAR_ARRAY_DIM_TYPE *objp)
{
	register int32_t *buf;

	int i;
	 if (!xdr_RAPVAR_SHORT_TYPE (xdrs, &objp->dimension))
		 return FALSE;
	 if (!xdr_vector (xdrs, (char *)objp->elements, 6,
		sizeof (RAPVAR_SHORT_TYPE), (xdrproc_t) xdr_RAPVAR_SHORT_TYPE))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_CONFDATA_TYPE (XDR *xdrs, RAPVAR_CONFDATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->cf1))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->cf4))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->cf6))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->cfx))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_EXTJOINT_TYPE (XDR *xdrs, RAPVAR_EXTJOINT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_a))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_b))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_c))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_d))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_e))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->eax_f))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_ORIENT_TYPE (XDR *xdrs, RAPVAR_ORIENT_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->q1))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->q2))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->q3))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->q4))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_POS_TYPE (XDR *xdrs, RAPVAR_POS_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->x))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->y))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->z))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_POSE_TYPE (XDR *xdrs, RAPVAR_POSE_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_POS_TYPE (xdrs, &objp->trans))
		 return FALSE;
	 if (!xdr_RAPVAR_ORIENT_TYPE (xdrs, &objp->rot))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_ROBTARGET_TYPE (XDR *xdrs, RAPVAR_ROBTARGET_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_POS_TYPE (xdrs, &objp->trans))
		 return FALSE;
	 if (!xdr_RAPVAR_ORIENT_TYPE (xdrs, &objp->rot))
		 return FALSE;
	 if (!xdr_RAPVAR_CONFDATA_TYPE (xdrs, &objp->robconf))
		 return FALSE;
	 if (!xdr_RAPVAR_EXTJOINT_TYPE (xdrs, &objp->extax))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_SPEEDDATA_TYPE (XDR *xdrs, RAPVAR_SPEEDDATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->v_tcp))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->v_ori))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->v_leax))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->v_reax))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_WOBJDATA_TYPE (XDR *xdrs, RAPVAR_WOBJDATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_BOOL_TYPE (xdrs, &objp->robhold))
		 return FALSE;
	 if (!xdr_RAPVAR_BOOL_TYPE (xdrs, &objp->ufprog))
		 return FALSE;
	 if (!xdr_RAPVAR_STRING_TYPE (xdrs, &objp->ufmec))
		 return FALSE;
	 if (!xdr_RAPVAR_POSE_TYPE (xdrs, &objp->uframe))
		 return FALSE;
	 if (!xdr_RAPVAR_POSE_TYPE (xdrs, &objp->oframe))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_ZONEDATA_TYPE (XDR *xdrs, RAPVAR_ZONEDATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_BOOL_TYPE (xdrs, &objp->finep))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->pzone_tcp))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->pzone_ori))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->pzone_eax))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->zone_ori))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->zone_leax))
		 return FALSE;
	 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->zone_reax))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_CURR_ROBTARGET_TYPE (XDR *xdrs, RAPVAR_CURR_ROBTARGET_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_ROBTARGET_TYPE (xdrs, &objp->robtarget))
		 return FALSE;
	 if (!xdr_RAPVAR_STRING_TYPE (xdrs, &objp->wobj))
		 return FALSE;
	 if (!xdr_RAPVAR_STRING_TYPE (xdrs, &objp->toolobj))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_DATA_TYPE (XDR *xdrs, RAPVAR_DATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAPVAR_NUMBER_TYPE (xdrs, &objp->var_type))
		 return FALSE;
	switch (objp->var_type) {
	case 1:
		 if (!xdr_RAPVAR_NUM_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.num))
			 return FALSE;
		break;
	case 2:
		 if (!xdr_RAP_LONG_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.raplong))
			 return FALSE;
		break;
	case 3:
		 if (!xdr_RAPVAR_STRING_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.charstring))
			 return FALSE;
		break;
	case 4:
		 if (!xdr_RAPVAR_BOOL_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.boolean))
			 return FALSE;
		break;
	case 7:
		 if (!xdr_RAPVAR_CONFDATA_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.confdata))
			 return FALSE;
		break;
	case 8:
		 if (!xdr_RAPVAR_EXTJOINT_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.extjoint))
			 return FALSE;
		break;
	case 11:
		 if (!xdr_RAPVAR_ORIENT_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.orient))
			 return FALSE;
		break;
	case 12:
		 if (!xdr_RAPVAR_POS_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.pos))
			 return FALSE;
		break;
	case 13:
		 if (!xdr_RAPVAR_POSE_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.pose))
			 return FALSE;
		break;
	case 15:
		 if (!xdr_RAPVAR_ROBTARGET_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.robtarget))
			 return FALSE;
		break;
	case 16:
		 if (!xdr_RAPVAR_SPEEDDATA_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.speeddata))
			 return FALSE;
		break;
	case 18:
		 if (!xdr_RAPVAR_WOBJDATA_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.wobjdata))
			 return FALSE;
		break;
	case 19:
		 if (!xdr_RAPVAR_ZONEDATA_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.zonedata))
			 return FALSE;
		break;
	case 26:
		 if (!xdr_RAPVAR_CURR_ROBTARGET_TYPE (xdrs, &objp->RAPVAR_DATA_TYPE_u.robposdata))
			 return FALSE;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

bool_t
xdr_RAPVAR_XDATA_TYPE (XDR *xdrs, RAPVAR_XDATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->datacfg.datacfg_val, (u_int *) &objp->datacfg.datacfg_len, ~0,
		sizeof (RAP_FLOAT_TYPE), (xdrproc_t) xdr_RAP_FLOAT_TYPE))
		 return FALSE;
	 if (!xdr_array (xdrs, (char **)&objp->numdata.numdata_val, (u_int *) &objp->numdata.numdata_len, ~0,
		sizeof (RAP_FLOAT_TYPE), (xdrproc_t) xdr_RAP_FLOAT_TYPE))
		 return FALSE;
	 if (!xdr_array (xdrs, (char **)&objp->strdata.strdata_val, (u_int *) &objp->strdata.strdata_len, ~0,
		sizeof (RAP_STRING_TYPE), (xdrproc_t) xdr_RAP_STRING_TYPE))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_ARRAY_DATA_TYPE (XDR *xdrs, RAPVAR_ARRAY_DATA_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->RAPVAR_ARRAY_DATA_TYPE_val, (u_int *) &objp->RAPVAR_ARRAY_DATA_TYPE_len, 500,
		sizeof (RAPVAR_XDATA_TYPE), (xdrproc_t) xdr_RAPVAR_XDATA_TYPE))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_REQ_TYPE (XDR *xdrs, RAPVAR_READ_NAME_REQ_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_NAME_TYPE (xdrs, &objp->var))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_RESPX_TYPE (XDR *xdrs, RAPVAR_READ_NAME_RESPX_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_STATUS_TYPE (xdrs, &objp->status))
		 return FALSE;
	switch (objp->status) {
	case 0:
		 if (!xdr_RAPVAR_DATA_TYPE (xdrs, &objp->RAPVAR_READ_NAME_RESPX_TYPE_u.data))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_RESP_TYPE (XDR *xdrs, RAPVAR_READ_NAME_RESP_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_READ_NAME_RESPX_TYPE (xdrs, &objp->resp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_WRITE_NAME_REQ_TYPE (XDR *xdrs, RAPVAR_WRITE_NAME_REQ_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_NAME_TYPE (xdrs, &objp->var))
		 return FALSE;
	 if (!xdr_RAPVAR_DATA_TYPE (xdrs, &objp->data))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_WRITE_NAME_RESP_TYPE (XDR *xdrs, RAPVAR_WRITE_NAME_RESP_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAP_STATUS_TYPE (xdrs, &objp->status))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_ARRAY_REQ_TYPE (XDR *xdrs, RAPVAR_READ_NAME_ARRAY_REQ_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_NAME_TYPE (xdrs, &objp->var))
		 return FALSE;
	 if (!xdr_RAPVAR_ARRAY_DIM_TYPE (xdrs, &objp->dim))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_ARRAY_RESPX_TYPE (XDR *xdrs, RAPVAR_READ_NAME_ARRAY_RESPX_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_STATUS_TYPE (xdrs, &objp->status))
		 return FALSE;
	switch (objp->status) {
	case 0:
		 if (!xdr_RAPVAR_ARRAY_DATA_TYPE (xdrs, &objp->RAPVAR_READ_NAME_ARRAY_RESPX_TYPE_u.data))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_RAPVAR_READ_NAME_ARRAY_RESP_TYPE (XDR *xdrs, RAPVAR_READ_NAME_ARRAY_RESP_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_READ_NAME_ARRAY_RESPX_TYPE (xdrs, &objp->resp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_WRITE_NAME_ARRAY_REQ_TYPE (XDR *xdrs, RAPVAR_WRITE_NAME_ARRAY_REQ_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAPVAR_NAME_TYPE (xdrs, &objp->var))
		 return FALSE;
	 if (!xdr_RAPVAR_ARRAY_DIM_TYPE (xdrs, &objp->dim))
		 return FALSE;
	 if (!xdr_RAPVAR_ARRAY_DATA_TYPE (xdrs, &objp->data))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_RAPVAR_WRITE_NAME_ARRAY_RESP_TYPE (XDR *xdrs, RAPVAR_WRITE_NAME_ARRAY_RESP_TYPE *objp)
{
	register int32_t *buf;

	 if (!xdr_RAP_HEAD_TYPE (xdrs, &objp->head))
		 return FALSE;
	 if (!xdr_RAP_STATUS_TYPE (xdrs, &objp->status))
		 return FALSE;
	return TRUE;
}
