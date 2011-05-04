/******************************************************************************
 *
 * Copyright(c) 2007 - 2010 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _RTL8192C_XMIT_C_
#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>
#include <wifi.h>
#include <osdep_intf.h>
#include <circ_buf.h>
#include <usb_ops.h>

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)
#error "Shall be Linux or Windows, but not both!\n"
#endif

u32 rtw_get_ff_hwaddr(struct xmit_frame	*pxmitframe)
{
	u32 addr;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;	
	
	if((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
		switch(pattrib->qsel)
		{
			case 0:
			case 3:
				addr = BE_QUEUE_INX;
			 	break;
			case 1:
			case 2:
				addr = BK_QUEUE_INX;
				break;				
			case 4:
			case 5:
				addr = VI_QUEUE_INX;
				break;		
			case 6:
			case 7:
				addr = VO_QUEUE_INX;
				break;

			default:
				addr = BE_QUEUE_INX;
				break;		
				
		}
		
	}	
	else if((pxmitframe->frame_tag&0x0f) == MGNT_FRAMETAG)
	{
		addr = MGT_QUEUE_INX;
	}
	else
	{
		addr = BE_QUEUE_INX;
	}

	return addr;

}

void count_tx_stats(_adapter *padapter, struct xmit_frame *pxmitframe, int sz)
{
	struct sta_info *psta = NULL;
	struct stainfo_stats *pstats = NULL;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
		pxmitpriv->tx_bytes += sz;
		pxmitpriv->NumTxOkInPeriod ++;

		psta = pxmitframe->attrib.psta;

		if(psta)
		{
			pstats = &psta->sta_stats;
#if USB_TX_AGGREGATION_92C
			pstats->tx_pkts += pxmitframe->agg_num;
#else
			pstats->tx_pkts++;
#endif
			pstats->tx_bytes += sz;
		}
	}
	
}

void rtw_do_queue_select(_adapter	*padapter, struct pkt_attrib *pattrib)
{
	unsigned int qsel;
		
	qsel = (uint)pattrib->priority;	
	RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("### rtw_do_queue_select priority=%d ,qsel = %d\n",pattrib->priority ,qsel));
	pattrib->qsel = qsel;

}

int urb_zero_packet_chk(_adapter *padapter, int sz)
{
	int blnSetTxDescOffset;
	struct dvobj_priv	*pdvobj = (struct dvobj_priv*)&padapter->dvobjpriv;	

	if ( pdvobj->ishighspeed )
	{
		if ( ( (sz + TXDESC_SIZE) % 512 ) == 0 ) {
			blnSetTxDescOffset = 1;
		} else {
			blnSetTxDescOffset = 0;
		}
	}
	else
	{
		if ( ( (sz + TXDESC_SIZE) % 64 ) == 0 ) 	{
			blnSetTxDescOffset = 1;
		} else {
			blnSetTxDescOffset = 0;
		}
	}
	
	return blnSetTxDescOffset;
	
}

void cal_txdesc_chksum(struct tx_desc	*ptxdesc)
{
		u16	*usPtr = (u16*)ptxdesc;
		u32 count = 16;		// (32 bytes / 2 bytes per XOR) => 16 times
		u32 index;
		u16 checksum = 0;

		//Clear first
		ptxdesc->txdw7 &= cpu_to_le32(0xffff0000);
	
		for(index = 0 ; index < count ; index++){
			checksum = checksum ^ le16_to_cpu(*(usPtr + index));
		}

		ptxdesc->txdw7 |= cpu_to_le32(0x0000ffff&checksum);	

}

void fill_txdesc_for_mp(struct xmit_frame *pxmitframe, struct tx_desc *ptxdesc)
{		
#ifdef CONFIG_MP_INCLUDED

		struct pkt_attrib	*pattrib = &pxmitframe->attrib;
		sint bmcst = IS_MCAST(pattrib->ra);

		if (pattrib->pctrl == 1) // mp tx packets
		{

			struct tx_desc txdesc, *ptxdesc_mp;
			struct pkt_file pktfile;

			ptxdesc_mp = &txdesc;
			_rtw_open_pktfile(pxmitframe->pkt, &pktfile);
			_rtw_pktfile_read(&pktfile, NULL, ETH_HLEN);
			_rtw_pktfile_read(&pktfile, (u8*)ptxdesc_mp, TXDESC_SIZE);

			//offset 8
			ptxdesc->txdw2 = cpu_to_le32(ptxdesc_mp->txdw2);
			if (bmcst) ptxdesc->txdw2 |= cpu_to_le32(BMC);
			ptxdesc->txdw2 |= cpu_to_le32(BK);	
			//RT_TRACE(_module_rtl871x_xmit_c_,_drv_alert_,("mp pkt offset8-txdesc=0x%8x\n", ptxdesc->txdw2));			

			ptxdesc->txdw4 = cpu_to_le32(ptxdesc_mp->txdw4);
			//RT_TRACE(_module_rtl871x_xmit_c_,_drv_alert_,("mp pkt offset16-txdesc=0x%8x\n", ptxdesc->txdw4));

			//offset 20
			ptxdesc->txdw5 = cpu_to_le32(ptxdesc_mp->txdw5);
			//RT_TRACE(_module_rtl871x_xmit_c_,_drv_alert_,("mp pkt offset20-txdesc=0x%8x\n", ptxdesc->txdw5));				

			pattrib->pctrl = 0;//reset to zero;				
		}
		
#endif
}

void fill_txdesc_sectype(struct pkt_attrib *pattrib, struct tx_desc *ptxdesc)
{
	if ((pattrib->encrypt > 0) && !pattrib->bswenc)
	{
		switch (pattrib->encrypt)
		{	
			//SEC_TYPE
			case _WEP40_:
			case _WEP104_:
					ptxdesc->txdw1 |= cpu_to_le32((0x01<<22)&0x00c00000);
					break;				
			case _TKIP_:
			case _TKIP_WTMIC_:	
					//ptxdesc->txdw1 |= cpu_to_le32((0x02<<22)&0x00c00000);
					ptxdesc->txdw1 |= cpu_to_le32((0x01<<22)&0x00c00000);
					break;
			case _AES_:
					ptxdesc->txdw1 |= cpu_to_le32((0x03<<22)&0x00c00000);
					break;
			case _NO_PRIVACY_:
			default:
					break;
		
		}
		
	}

}

void fill_txdesc_vcs(struct pkt_attrib *pattrib, u32 *pdw)
{
	//DBG_8192C("cvs_mode=%d\n", pattrib->vcs_mode);	

	switch(pattrib->vcs_mode)
	{
		case RTS_CTS:
			*pdw |= cpu_to_le32(BIT(12));
			break;
		case CTS_TO_SELF:
			*pdw |= cpu_to_le32(BIT(11));
			break;
		case NONE_VCS:
		default:
			break;		
	}

	if(pattrib->vcs_mode)
	{
		*pdw |= cpu_to_le32(BIT(13));//ENABLE HW RTS
	}

}

void fill_txdesc_phy(struct pkt_attrib *pattrib, u32 *pdw)
{
	//DBG_8192C("bwmode=%d, ch_off=%d\n", pattrib->bwmode, pattrib->ch_offset);

	if(pattrib->ht_en)
	{
		*pdw |= (pattrib->bwmode&HT_CHANNEL_WIDTH_40)?	cpu_to_le32(BIT(25)):0;

		if(pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_LOWER)
			*pdw |= cpu_to_le32((0x01<<20)&0x003f0000);
		else if(pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_UPPER)
			*pdw |= cpu_to_le32((0x02<<20)&0x003f0000);
		else if(pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_DONT_CARE)
			*pdw |= 0;
		else
			*pdw |= cpu_to_le32((0x03<<20)&0x003f0000);
	}
}

#if USB_TX_AGGREGATION_92C
static void _update_txdesc(struct xmit_frame *pxmitframe, u8 *pmem, int sz)
{
	uint qsel;

	_adapter		*padapter = pxmitframe->padapter;
	struct ht_priv		*phtpriv = &padapter->mlmepriv.htpriv;
	struct mlme_ext_info	*pmlmeinfo = &padapter->mlmeextpriv.mlmext_info;

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	sint bmcst = IS_MCAST(pattrib->ra);

	struct tx_desc		*ptxdesc = (struct tx_desc*)pmem;


	_rtw_memset(ptxdesc, 0, sizeof(struct tx_desc));
	
	//4 offset 0
	ptxdesc->txdw0 |= cpu_to_le32(sz & 0x0000ffff);
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);
	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE + OFFSET_SZ) << OFFSET_SHT) & 0x00ff0000);//32 bytes for TX Desc
	
	if (bmcst) ptxdesc->txdw0 |= cpu_to_le32(BIT(24));

	RT_TRACE(_module_rtl871x_xmit_c_, _drv_info_,
		 ("rtw_update_txdesc: offset0=0x%08x\n", ptxdesc->txdw0));

	//4 offset 4
	// pkt_offset, unit:8 bytes padding
	if (pxmitframe->pkt_offset > 0)
		ptxdesc->txdw1 |= cpu_to_le32((pxmitframe->pkt_offset << 26) & 0x7c000000);

#if USB_TX_AGGREGATION_92C
	if (pxmitframe->agg_num > 0)
		ptxdesc->txdw5 |= cpu_to_le32((pxmitframe->agg_num << 24) & 0xff000000);
#endif

	if (pxmitframe->frame_tag == DATA_FRAMETAG)
	{
		//4 offset 4
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id & 0x1f);//CAM_ID(MAC_ID)

		qsel = (uint)(pattrib->qsel & 0x0000001f);
		ptxdesc->txdw1 |= cpu_to_le32((qsel << QSEL_SHT) & 0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid << 16) & 0x000f0000);

		fill_txdesc_sectype(pattrib, ptxdesc);

		if(pattrib->ampdu_en==_TRUE)
			ptxdesc->txdw1 |= cpu_to_le32(BIT(5));//AGG EN
		else
			ptxdesc->txdw1 |= cpu_to_le32(BIT(6));//AGG BK


		//4 offset 8


		//4 offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum << 16) & 0xffff0000);


		//4 offset 16 , offset 20
		if (pattrib->qos_en)
			ptxdesc->txdw4 |= cpu_to_le32(BIT(6));//QoS

		if ((pattrib->ether_type != 0x888e) &&
		    (pattrib->ether_type != 0x0806) &&
		    (pattrib->dhcp_pkt != 1))
		{
			//Non EAP & ARP & DHCP type data packet

			fill_txdesc_vcs(pattrib, &ptxdesc->txdw4);
			fill_txdesc_phy(pattrib, &ptxdesc->txdw4);

			
			ptxdesc->txdw4 |= cpu_to_le32(0x00000008);//RTS Rate=24M
			ptxdesc->txdw5 |= cpu_to_le32(0x0001ff00);
			ptxdesc->txdw5 |= cpu_to_le32(0x0000000b);//DataRate - 54M

			if (0)//for driver dbg
			{
				ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate

				if (pattrib->ht_en)
					ptxdesc->txdw5 |= cpu_to_le32(BIT(6));//SGI

				ptxdesc->txdw5 |= cpu_to_le32(0x00000013);//init rate - mcs7
			}
		}
		else
		{
			// EAP data packet and ARP packet.
			// Use the 1M data rate to send the EAP/ARP packet.
			// This will maybe make the handshake smooth.

			ptxdesc->txdw1 |= cpu_to_le32(BIT(6));//AGG BK
		   	ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate
		}


		//4 offset 24
#ifdef CONFIG_RTL8712_TCP_CSUM_OFFLOAD_TX
		if (pattrib->hw_tcp_csum == 1) {
			// ptxdesc->txdw6 = 0; // clear TCP_CHECKSUM and IP_CHECKSUM. It's zero already!!
			u8 ip_hdr_offset = 32 + pattrib->hdrlen + pattrib->iv_len + 8;
			ptxdesc->txdw7 = (1 << 31) | (ip_hdr_offset << 16);
			printk("ptxdesc->txdw7 = %08x\n", ptxdesc->txdw7);
		}
#endif

		fill_txdesc_for_mp(pxmitframe, ptxdesc);
	}
	else if(pxmitframe->frame_tag == MGNT_FRAMETAG)
	{
		//printk("pxmitframe->frame_tag == MGNT_FRAMETAG\n");	

		//4 offset 4
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id & 0x1f);//(MAC_ID)

		qsel = (uint)(pattrib->qsel&0x0000001f);
		ptxdesc->txdw1 |= cpu_to_le32((qsel << QSEL_SHT) & 0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid<< 16) & 0x000f0000);

		//fill_txdesc_sectype(pattrib, ptxdesc);


		//4 offset 8


		//4 offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<16)&0xffff0000);


		//4 offset 16
		ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate


		//4 offset 20
	}
	else if(pxmitframe->frame_tag == TXAGG_FRAMETAG)
	{
		DBG_8192C("pxmitframe->frame_tag == TXAGG_FRAMETAG\n");
	}
	else
	{
		DBG_8192C("pxmitframe->frame_tag = %d\n", pxmitframe->frame_tag);

		//4 offset 4
		ptxdesc->txdw1 |= cpu_to_le32((4)&0x1f);//CAM_ID(MAC_ID)

		ptxdesc->txdw1 |= cpu_to_le32((6<< 16) & 0x000f0000);//raid


		//4 offset 8


		//4 offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum << 16) & 0xffff0000);


		//4 offset 16
		ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate


		//4 offset 20
	}

	// 2009.11.05. tynli_test. Suggested by SD4 Filen for FW LPS.
	// (1) The sequence number of each non-Qos frame / broadcast / multicast /
	// mgnt frame should be controled by Hw because Fw will also send null data
	// which we cannot control when Fw LPS enable.
	// --> default enable non-Qos data sequense number. 2010.06.23. by tynli.
	// (2) Enable HW SEQ control for beacon packet, because we use Hw beacon.
	// (3) Use HW Qos SEQ to control the seq num of Ext port non-Qos packets.
	// 2010.06.23. Added by tynli.
	if(!pattrib->qos_en)
	{		
		ptxdesc->txdw4 |= cpu_to_le32(BIT(7)); // Hw set sequence number
		ptxdesc->txdw3 |= cpu_to_le32((8 <<28)); //set bit3 to 1. Suugested by TimChen. 2009.12.29.
	}

	cal_txdesc_chksum(ptxdesc);
}
#endif

s32 rtw_update_txdesc(struct xmit_frame *pxmitframe, u32 *pmem, s32 sz)
{
	int pull=0;
	uint qsel;
	_adapter		*padapter = pxmitframe->padapter;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;		
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct tx_desc	*ptxdesc = (struct tx_desc *)pmem;
	sint bmcst = IS_MCAST(pattrib->ra);
	struct ht_priv *phtpriv = &pmlmepriv->htpriv;
	struct mlme_ext_info *pmlmeinfo = &padapter->mlmeextpriv.mlmext_info;

	if(urb_zero_packet_chk(padapter, sz)==0)
	{
		ptxdesc = (struct tx_desc *)(pmem+(PACKET_OFFSET_SZ>>2));
		pull = 1;
	}
	
		
	_rtw_memset(ptxdesc, 0, sizeof(struct tx_desc));
	
	//offset 0
	ptxdesc->txdw0 |= cpu_to_le32(sz&0x0000ffff);
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);
	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE+OFFSET_SZ)<<OFFSET_SHT)&0x00ff0000);//32 bytes for TX Desc
	
	if(bmcst)	
	{
		ptxdesc->txdw0 |= cpu_to_le32(BIT(24));
	}	

	RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("offset0-txdesc=0x%x\n", ptxdesc->txdw0));

	//offset 4
	if(!pull) ptxdesc->txdw1 |= cpu_to_le32((0x01<<26)&0xff000000);//pkt_offset, unit:8 bytes padding


	if((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
		//printk("pxmitframe->frame_tag == DATA_FRAMETAG\n");			

		//offset 4
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id & 0x1f);//(MAC_ID)

		qsel = (uint)(pattrib->qsel & 0x0000001f);
		ptxdesc->txdw1 |= cpu_to_le32((qsel << QSEL_SHT) & 0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid<< 16) & 0x000f0000);

		fill_txdesc_sectype(pattrib, ptxdesc);

	
		if(pattrib->ampdu_en==_TRUE)
			ptxdesc->txdw1 |= cpu_to_le32(BIT(5));//AGG EN
		else
			ptxdesc->txdw1 |= cpu_to_le32(BIT(6));//AGG BK
		
		//offset 8
		

		//offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<16)&0xffff0000);


		//offset 16 , offset 20
		if (pattrib->qos_en)
			ptxdesc->txdw4 |= cpu_to_le32(BIT(6));//QoS

		if ((pattrib->ether_type != 0x888e) && (pattrib->ether_type != 0x0806) && (pattrib->dhcp_pkt != 1))
		{
              	//Non EAP & ARP & DHCP type data packet
              	
			fill_txdesc_vcs(pattrib, &ptxdesc->txdw4);
			fill_txdesc_phy(pattrib, &ptxdesc->txdw4);

			ptxdesc->txdw4 |= cpu_to_le32(0x00000008);//RTS Rate=24M
			ptxdesc->txdw5 |= cpu_to_le32(0x0001ff00);//
			ptxdesc->txdw5 |= cpu_to_le32(0x0000000b);//DataRate - 54M
				
              	if(0)//for driver dbg
			{
				ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate
				
				if(pattrib->ht_en)
					ptxdesc->txdw5 |= cpu_to_le32(BIT(6));//SGI

				ptxdesc->txdw5 |= cpu_to_le32(0x00000013);//init rate - mcs7
			}

		}
		else
		{
			// EAP data packet and ARP packet.
			// Use the 1M data rate to send the EAP/ARP packet.
			// This will maybe make the handshake smooth.

			ptxdesc->txdw1 |= cpu_to_le32(BIT(6));//AGG BK
			
		   	ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate

		}
		
		//offset 24

#ifdef CONFIG_RTL8712_TCP_CSUM_OFFLOAD_TX
		if ( pattrib->hw_tcp_csum == 1 ) {
			// ptxdesc->txdw6 = 0; // clear TCP_CHECKSUM and IP_CHECKSUM. It's zero already!!
			u8 ip_hdr_offset = 32 + pattrib->hdrlen + pattrib->iv_len + 8;
			ptxdesc->txdw7 = (1 << 31) | (ip_hdr_offset << 16);
			printk("ptxdesc->txdw7 = %08x\n", ptxdesc->txdw7);
		}
#endif

		
		fill_txdesc_for_mp(pxmitframe, ptxdesc);

                  

	}
	else if((pxmitframe->frame_tag&0x0f)== MGNT_FRAMETAG)
	{
		//printk("pxmitframe->frame_tag == MGNT_FRAMETAG\n");	
		
		//offset 4		
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id & 0x1f);//(MAC_ID)
		
		qsel = (uint)(pattrib->qsel&0x0000001f);
		ptxdesc->txdw1 |= cpu_to_le32((qsel<<QSEL_SHT)&0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid<< 16) & 0x000f0000);
		
		//fill_txdesc_sectype(pattrib, ptxdesc);
		
		//offset 8		

		//offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<16)&0xffff0000);
		
		//offset 16
		ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate
		
		//offset 20
		
	}
	else if((pxmitframe->frame_tag&0x0f) == TXAGG_FRAMETAG)
	{
		DBG_8192C("pxmitframe->frame_tag == TXAGG_FRAMETAG\n");
	}
	else
	{
		DBG_8192C("pxmitframe->frame_tag = %d\n", pxmitframe->frame_tag);
		
		//offset 4	
		ptxdesc->txdw1 |= cpu_to_le32((4)&0x1f);//CAM_ID(MAC_ID)
		
		ptxdesc->txdw1 |= cpu_to_le32((6<< 16) & 0x000f0000);//raid
		
		//offset 8		

		//offset 12
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<16)&0xffff0000);
		
		//offset 16
		ptxdesc->txdw4 |= cpu_to_le32(BIT(8));//driver uses rate
		
		//offset 20
	}

	// 2009.11.05. tynli_test. Suggested by SD4 Filen for FW LPS.
	// (1) The sequence number of each non-Qos frame / broadcast / multicast /
	// mgnt frame should be controled by Hw because Fw will also send null data
	// which we cannot control when Fw LPS enable.
	// --> default enable non-Qos data sequense number. 2010.06.23. by tynli.
	// (2) Enable HW SEQ control for beacon packet, because we use Hw beacon.
	// (3) Use HW Qos SEQ to control the seq num of Ext port non-Qos packets.
	// 2010.06.23. Added by tynli.
	if(!pattrib->qos_en)
	{		
		ptxdesc->txdw4 |= cpu_to_le32(BIT(7)); // Hw set sequence number
		ptxdesc->txdw3 |= cpu_to_le32((8 <<28)); //set bit3 to 1. Suugested by TimChen. 2009.12.29.
	}

	cal_txdesc_chksum(ptxdesc);
		
	return pull;
		
}

#if USB_TX_AGGREGATION_92C
static s32 xmitframe_need_length(struct xmit_frame *pxmitframe)
{
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	int len = 0;

	// no consider fragement
	len = pattrib->hdrlen + pattrib->iv_len +
		SNAP_SIZE + sizeof(u16) +
		pattrib->pktlen +
		((pattrib->bswenc) ? pattrib->icv_len : 0);

	if(pattrib->encrypt ==_TKIP_)
		len += 8;

	return len;
}

#define IDEA_CONDITION 1	// check all packets before enqueue
s32 rtw_xmitframe_complete(_adapter *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_frame *pfirstframe = NULL;

	// aggregate variable
//	struct hw_xmit *phwxmit;
	struct sta_info *psta = NULL;
	struct tx_servq *ptxservq = NULL;

	_irqL irqL;
	_list *xmitframe_plist = NULL, *xmitframe_phead = NULL;

	u32 pbuf;	// next pkt address
	u32 pbuf_tail;	// last pkt tail
	int len;	// packet length, except TXDESC_SIZE and PKT_OFFSET

	unsigned int bulkSize = padapter->halpriv.UsbBulkOutSize;
	int descCount;
	u32 bulkPtr;

	// dump frame variable
	u32 ff_hwaddr;

#ifndef IDEA_CONDITION
	int res = _SUCCESS;
#endif

	RT_TRACE(_module_rtl8192c_xmit_c_, _drv_info_, ("+rtw_xmitframe_complete\n"));


	// check xmitbuffer is ok
	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL) return _FALSE;
	}


	//3 1. pick up first frame
	do {
		rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			
		pxmitframe = rtw_dequeue_xframe(pxmitpriv, pxmitbuf->flags);
		if (pxmitframe == NULL) {
			// no more xmit frame, release xmit buffer
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}


#ifndef IDEA_CONDITION
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("rtw_xmitframe_complete: frame tag(%d) is not DATA_FRAMETAG(%d)!\n",
				  pxmitframe->frame_tag, DATA_FRAMETAG));
//			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}

		// TID 0~15
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("rtw_xmitframe_complete: TID(%d) should be 0~15!\n",
				  pxmitframe->attrib.priority));
//			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}
#endif

		pxmitframe->pxmitbuf = pxmitbuf;
		pxmitframe->buf_addr = pxmitbuf->pbuf;
		pxmitbuf->priv_data = pxmitframe;

		pxmitframe->agg_num = 1; // first frame of aggregation
		pxmitframe->pkt_offset = 1; // first frame of aggregation, reserve offset

#ifdef IDEA_CONDITION
		rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
#else
		res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
		if (res == _FALSE) {
//			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}
#endif

		// always return ndis_packet after rtw_xmitframe_coalesce
		rtw_os_xmit_complete(padapter, pxmitframe);

		if ((pxmitframe->attrib.ether_type != 0x0806) &&
		    (pxmitframe->attrib.ether_type != 0x888e) &&
		    (pxmitframe->attrib.dhcp_pkt != 1))
			rtw_issue_addbareq_cmd(padapter, pxmitframe);

		break;
	} while (1);

	//3 2. aggregate same priority and same DA(AP or STA) frames
	pfirstframe = pxmitframe;
	len = xmitframe_need_length(pfirstframe) + TXDESC_OFFSET;
	pbuf_tail = len;
	pbuf = _RND8(pbuf_tail);

	// check pkt amount in one bluk
	descCount = 0;
	bulkPtr = bulkSize;
	if (pbuf < bulkPtr)
		descCount++;
	else {
		descCount = 0;
		bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize; // round to next bulkSize
	}

	// dequeue same priority packet from station tx queue
	psta = pfirstframe->attrib.psta;
	switch (pfirstframe->attrib.priority) {
		case 1:
		case 2:
			ptxservq = &(psta->sta_xmitpriv.bk_q);
//			phwxmit = pxmitpriv->hwxmits + 3;
			break;

		case 4:
		case 5:
			ptxservq = &(psta->sta_xmitpriv.vi_q);
//			phwxmit = pxmitpriv->hwxmits + 1;
			break;

		case 6:
		case 7:
			ptxservq = &(psta->sta_xmitpriv.vo_q);
//			phwxmit = pxmitpriv->hwxmits;
			break;

		case 0:
		case 3:
		default:
			ptxservq = &(psta->sta_xmitpriv.be_q);
//			phwxmit = pxmitpriv->hwxmits + 2;
			break;
	}

	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	xmitframe_phead = get_list_head(&ptxservq->sta_pending);
	xmitframe_plist = get_next(xmitframe_phead);
	while (rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist) == _FALSE)
	{
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);
		xmitframe_plist = get_next(xmitframe_plist);

		len = xmitframe_need_length(pxmitframe) + TXDESC_SIZE; // no offset
		if (pbuf + len > MAX_XMITBUF_SZ) break;

		list_delete(&pxmitframe->list);
		ptxservq->qcnt--;
//		phwxmit->accnt--;	// ?
//		phwxmit->txcmdcnt++;	// ?

#ifndef IDEA_CONDITION
		// suppose only data frames would be in queue
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("rtw_xmitframe_complete: frame tag(%d) is not DATA_FRAMETAG(%d)!\n",
				  pxmitframe->frame_tag, DATA_FRAMETAG));
			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}

		// TID 0~15
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("rtw_xmitframe_complete: TID(%d) should be 0~15!\n",
				  pxmitframe->attrib.priority));
			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}
#endif

//		pxmitframe->pxmitbuf = pxmitbuf;
		pxmitframe->buf_addr = pxmitbuf->pbuf + pbuf;

		pxmitframe->agg_num = 0; // not first frame of aggregation
		pxmitframe->pkt_offset = 0; // not first frame of aggregation, no need to reserve offset

#ifdef IDEA_CONDITION
		rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
#else
		res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
		if (res == _FALSE) {
			rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
			continue;
		}
#endif

		// always return ndis_packet after rtw_xmitframe_coalesce
		rtw_os_xmit_complete(padapter, pxmitframe);

		if ((pxmitframe->attrib.ether_type != 0x0806) &&
		    (pxmitframe->attrib.ether_type != 0x888e) &&
		    (pxmitframe->attrib.dhcp_pkt != 1))
			rtw_issue_addbareq_cmd(padapter, pxmitframe);

		// (len - TXDESC_SIZE) == pxmitframe->attrib.last_txcmdsz
		_update_txdesc(pxmitframe, pxmitframe->buf_addr, pxmitframe->attrib.last_txcmdsz);

		// don't need xmitframe any more
		rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);

		// handle pointer and stop condition
		pbuf_tail = pbuf + len;
		pbuf = _RND8(pbuf_tail);

		pfirstframe->agg_num++;
		if (MAX_TX_AGG_PACKET_NUMBER == pfirstframe->agg_num)
			break;

		if (pbuf < bulkPtr) {
			descCount++;
			if (descCount == padapter->halpriv.UsbTxAggDescNum)
				break;
		} else {
			descCount = 0;
			bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize;
		}
	}
	
	if (_rtw_queue_empty(&ptxservq->sta_pending) == _TRUE)
		list_delete(&ptxservq->tx_pending);

	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	//3 3. update first frame txdesc
	if ((pbuf_tail % bulkSize) == 0) {
		// remove pkt_offset
		pbuf_tail -= PACKET_OFFSET_SZ;
		pfirstframe->buf_addr += PACKET_OFFSET_SZ;
		pfirstframe->pkt_offset = 0;
	}
	_update_txdesc(pfirstframe, pfirstframe->buf_addr, pfirstframe->attrib.last_txcmdsz);

	//3 4. write xmit buffer to USB FIFO
	ff_hwaddr = rtw_get_ff_hwaddr(pfirstframe);

	// xmit address == ((xmit_frame*)pxmitbuf->priv_data)->buf_addr
	rtw_write_port(padapter, ff_hwaddr, pbuf_tail, (u8*)pxmitbuf);


	//3 5. update statisitc
	pbuf_tail -= (pfirstframe->agg_num * TXDESC_SIZE);
	if (pfirstframe->pkt_offset == 1) pbuf_tail -= PACKET_OFFSET_SZ;
	
	count_tx_stats(padapter, pfirstframe, pbuf_tail);

	rtw_free_xmitframe_ex(pxmitpriv, pfirstframe);

	return _TRUE;
}

#else

s32 rtw_xmitframe_complete(_adapter *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	struct xmit_frame *pxmitframe=NULL;	
	int res=_SUCCESS, xcnt = 0;


	RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_xmitframe_complete()\n"));

	if(pxmitbuf==NULL)
	{
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);		
		if(!pxmitbuf)
		{
			return _FALSE;
		}			
	}	


	do
	{		
		pxmitframe =  rtw_dequeue_xframe(pxmitpriv, pxmitbuf->flags);
		
		if(pxmitframe)
		{
			pxmitframe->pxmitbuf = pxmitbuf;				

			pxmitframe->buf_addr = pxmitbuf->pbuf;

			pxmitbuf->priv_data = pxmitframe;	

			if((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
			{	
				if(pxmitframe->attrib.priority<=15)//TID0~15
				{
					res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
				}	
							
				rtw_os_xmit_complete(padapter, pxmitframe);//always return ndis_packet after rtw_xmitframe_coalesce 			
			}	

				
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_xmitframe_complete(): rtw_dump_xframe\n"));

			
			if(res == _SUCCESS)
			{
				rtw_dump_xframe(padapter, pxmitframe);		 
			}
			else
			{
				rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
				rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);	
			}
	 			 		
			xcnt++;
			
		}
		else
		{			
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}

		break;
		
	}while(0/*xcnt < (NR_XMITFRAME >> 3)*/);

	return _TRUE;
	
}
#endif

void rtw_dump_xframe(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	int t, sz, w_sz, pull=0;
	u8 *mem_addr;
	u32 ff_hwaddr;
	struct xmit_buf *pxmitbuf = pxmitframe->pxmitbuf;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
	    (pxmitframe->attrib.ether_type != 0x0806) &&
	    (pxmitframe->attrib.ether_type != 0x888e) &&
	    (pxmitframe->attrib.dhcp_pkt != 1))
		rtw_issue_addbareq_cmd(padapter, pxmitframe);

	mem_addr = pxmitframe->buf_addr;

       RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_dump_xframe()\n"));
	
	for (t = 0; t < pattrib->nr_frags; t++)
	{
		if (t != (pattrib->nr_frags - 1))
		{
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("pattrib->nr_frags=%d\n", pattrib->nr_frags));

			sz = pxmitpriv->frag_len;
			sz = sz - 4 - (psecuritypriv->sw_encrypt ? 0 : pattrib->icv_len);					
		}
		else //no frag
		{
			sz = pattrib->last_txcmdsz;
		}

		pull = rtw_update_txdesc(pxmitframe, (uint*)mem_addr, sz);
		
		if(pull)
		{
			mem_addr += PACKET_OFFSET_SZ; //pull txdesc head
			
			//pxmitbuf ->pbuf = mem_addr;			
			pxmitframe->buf_addr = mem_addr;

			w_sz = sz + TXDESC_SIZE;
		}
		else
		{
			w_sz = sz + TXDESC_SIZE + PACKET_OFFSET_SZ;
		}	

		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);
		
		rtw_write_port(padapter, ff_hwaddr, w_sz, (unsigned char*)pxmitbuf);

		count_tx_stats(padapter, pxmitframe, sz);


		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_write_port, w_sz=%d\n", w_sz));
		//printk("rtw_write_port, w_sz=%d, sz=%d, txdesc_sz=%d, tid=%d\n", w_sz, sz, w_sz-sz, pattrib->priority);      

		mem_addr += w_sz;

		mem_addr = (u8 *)RND4(((SIZE_PTR)(mem_addr)));

	}
	
	rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
	
}

s32 xmitframe_direct(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	s32 res = _SUCCESS;


	res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
	if (res == _SUCCESS) {
		rtw_dump_xframe(padapter, pxmitframe);
	}
	
	return res;
}

/*
 * Return
 *	_TRUE	dump packet directly
 *	_FALSE	enqueue packet
 */
s32 pre_xmitframe(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	_irqL irqL;
	s32 res;
	struct xmit_buf *pxmitbuf = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;	




	rtw_do_queue_select(padapter, pattrib);
	
	_enter_critical_bh(&pxmitpriv->lock, &irqL);
	
	if (rtw_txframes_sta_ac_pending(padapter, pattrib) > 0)
		goto enqueue;

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE)
		goto enqueue;

#ifdef CONFIG_AP_MODE
	if(xmitframe_enqueue_for_sleeping_sta(padapter, pxmitframe) == _TRUE)
	{
		_exit_critical_bh(&pxmitpriv->lock, &irqL);
		return _FALSE;
	}
#endif

	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);	
	if (pxmitbuf == NULL)
		goto enqueue;
	
        _exit_critical_bh(&pxmitpriv->lock, &irqL);

	pxmitframe->pxmitbuf = pxmitbuf;
	pxmitframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pxmitframe;

	if (xmitframe_direct(padapter, pxmitframe) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);
	}

	return _TRUE;

enqueue:
	res = xmitframe_enqueue(padapter, pxmitframe);
	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	if (res != _SUCCESS) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("pre_xmitframe: enqueue xmitframe fail\n"));
		rtw_free_xmitframe_ex(pxmitpriv, pxmitframe);

		// Trick, make the statistics correct
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
		return _TRUE;
	}

	return _FALSE;
}

/*
 * Return
 *	_TRUE	dump packet directly ok
 *	_FALSE	temporary can't transmit packets to hardware
 */
s32 hal_xmit(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	return pre_xmitframe(padapter, pxmitframe);
}

