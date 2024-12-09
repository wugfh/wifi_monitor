#include <string.h>
#include <stdio.h>
#include "pack_cap.h"
#include "ieee80211_parse.h"
#include "util.h"

/*
 * Indices are:
 *
 *	the MCS index (0-76);
 *
 *	0 for 20 MHz, 1 for 40 MHz;
 *
 *	0 for a long guard interval, 1 for a short guard interval.
 */
static const float ieee80211_float_htrates[MAX_MCS_INDEX+1][2][2] = {
	/* MCS  0  */
	{	/* 20 Mhz */ {    6.5f,		/* SGI */    7.2f, },
		/* 40 Mhz */ {   13.5f,		/* SGI */   15.0f, },
	},

	/* MCS  1  */
	{	/* 20 Mhz */ {   13.0f,		/* SGI */   14.4f, },
		/* 40 Mhz */ {   27.0f,		/* SGI */   30.0f, },
	},

	/* MCS  2  */
	{	/* 20 Mhz */ {   19.5f,		/* SGI */   21.7f, },
		/* 40 Mhz */ {   40.5f,		/* SGI */   45.0f, },
	},

	/* MCS  3  */
	{	/* 20 Mhz */ {   26.0f,		/* SGI */   28.9f, },
		/* 40 Mhz */ {   54.0f,		/* SGI */   60.0f, },
	},

	/* MCS  4  */
	{	/* 20 Mhz */ {   39.0f,		/* SGI */   43.3f, },
		/* 40 Mhz */ {   81.0f,		/* SGI */   90.0f, },
	},

	/* MCS  5  */
	{	/* 20 Mhz */ {   52.0f,		/* SGI */   57.8f, },
		/* 40 Mhz */ {  108.0f,		/* SGI */  120.0f, },
	},

	/* MCS  6  */
	{	/* 20 Mhz */ {   58.5f,		/* SGI */   65.0f, },
		/* 40 Mhz */ {  121.5f,		/* SGI */  135.0f, },
	},

	/* MCS  7  */
	{	/* 20 Mhz */ {   65.0f,		/* SGI */   72.2f, },
		/* 40 Mhz */ {   135.0f,	/* SGI */  150.0f, },
	},

	/* MCS  8  */
	{	/* 20 Mhz */ {   13.0f,		/* SGI */   14.4f, },
		/* 40 Mhz */ {   27.0f,		/* SGI */   30.0f, },
	},

	/* MCS  9  */
	{	/* 20 Mhz */ {   26.0f,		/* SGI */   28.9f, },
		/* 40 Mhz */ {   54.0f,		/* SGI */   60.0f, },
	},

	/* MCS 10  */
	{	/* 20 Mhz */ {   39.0f,		/* SGI */   43.3f, },
		/* 40 Mhz */ {   81.0f,		/* SGI */   90.0f, },
	},

	/* MCS 11  */
	{	/* 20 Mhz */ {   52.0f,		/* SGI */   57.8f, },
		/* 40 Mhz */ {  108.0f,		/* SGI */  120.0f, },
	},

	/* MCS 12  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 13  */
	{	/* 20 Mhz */ {  104.0f,		/* SGI */  115.6f, },
		/* 40 Mhz */ {  216.0f,		/* SGI */  240.0f, },
	},

	/* MCS 14  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 15  */
	{	/* 20 Mhz */ {  130.0f,		/* SGI */  144.4f, },
		/* 40 Mhz */ {  270.0f,		/* SGI */  300.0f, },
	},

	/* MCS 16  */
	{	/* 20 Mhz */ {   19.5f,		/* SGI */   21.7f, },
		/* 40 Mhz */ {   40.5f,		/* SGI */   45.0f, },
	},

	/* MCS 17  */
	{	/* 20 Mhz */ {   39.0f,		/* SGI */   43.3f, },
		/* 40 Mhz */ {   81.0f,		/* SGI */   90.0f, },
	},

	/* MCS 18  */
	{	/* 20 Mhz */ {   58.5f,		/* SGI */   65.0f, },
		/* 40 Mhz */ {  121.5f,		/* SGI */  135.0f, },
	},

	/* MCS 19  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 20  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 21  */
	{	/* 20 Mhz */ {  156.0f,		/* SGI */  173.3f, },
		/* 40 Mhz */ {  324.0f,		/* SGI */  360.0f, },
	},

	/* MCS 22  */
	{	/* 20 Mhz */ {  175.5f,		/* SGI */  195.0f, },
		/* 40 Mhz */ {  364.5f,		/* SGI */  405.0f, },
	},

	/* MCS 23  */
	{	/* 20 Mhz */ {  195.0f,		/* SGI */  216.7f, },
		/* 40 Mhz */ {  405.0f,		/* SGI */  450.0f, },
	},

	/* MCS 24  */
	{	/* 20 Mhz */ {   26.0f,		/* SGI */   28.9f, },
		/* 40 Mhz */ {   54.0f,		/* SGI */   60.0f, },
	},

	/* MCS 25  */
	{	/* 20 Mhz */ {   52.0f,		/* SGI */   57.8f, },
		/* 40 Mhz */ {  108.0f,		/* SGI */  120.0f, },
	},

	/* MCS 26  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 27  */
	{	/* 20 Mhz */ {  104.0f,		/* SGI */  115.6f, },
		/* 40 Mhz */ {  216.0f,		/* SGI */  240.0f, },
	},

	/* MCS 28  */
	{	/* 20 Mhz */ {  156.0f,		/* SGI */  173.3f, },
		/* 40 Mhz */ {  324.0f,		/* SGI */  360.0f, },
	},

	/* MCS 29  */
	{	/* 20 Mhz */ {  208.0f,		/* SGI */  231.1f, },
		/* 40 Mhz */ {  432.0f,		/* SGI */  480.0f, },
	},

	/* MCS 30  */
	{	/* 20 Mhz */ {  234.0f,		/* SGI */  260.0f, },
		/* 40 Mhz */ {  486.0f,		/* SGI */  540.0f, },
	},

	/* MCS 31  */
	{	/* 20 Mhz */ {  260.0f,		/* SGI */  288.9f, },
		/* 40 Mhz */ {  540.0f,		/* SGI */  600.0f, },
	},

	/* MCS 32  */
	{	/* 20 Mhz */ {    0.0f,		/* SGI */    0.0f, }, /* not valid */
		/* 40 Mhz */ {    6.0f,		/* SGI */    6.7f, },
	},

	/* MCS 33  */
	{	/* 20 Mhz */ {   39.0f,		/* SGI */   43.3f, },
		/* 40 Mhz */ {   81.0f,		/* SGI */   90.0f, },
	},

	/* MCS 34  */
	{	/* 20 Mhz */ {   52.0f,		/* SGI */   57.8f, },
		/* 40 Mhz */ {  108.0f,		/* SGI */  120.0f, },
	},

	/* MCS 35  */
	{	/* 20 Mhz */ {   65.0f,		/* SGI */   72.2f, },
		/* 40 Mhz */ {  135.0f,		/* SGI */  150.0f, },
	},

	/* MCS 36  */
	{	/* 20 Mhz */ {   58.5f,		/* SGI */   65.0f, },
		/* 40 Mhz */ {  121.5f,		/* SGI */  135.0f, },
	},

	/* MCS 37  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 38  */
	{	/* 20 Mhz */ {   97.5f,		/* SGI */  108.3f, },
		/* 40 Mhz */ {  202.5f,		/* SGI */  225.0f, },
	},

	/* MCS 39  */
	{	/* 20 Mhz */ {   52.0f,		/* SGI */   57.8f, },
		/* 40 Mhz */ {  108.0f,		/* SGI */  120.0f, },
	},

	/* MCS 40  */
	{	/* 20 Mhz */ {   65.0f,		/* SGI */   72.2f, },
		/* 40 Mhz */ {  135.0f,		/* SGI */  150.0f, },
	},

	/* MCS 41  */
	{	/* 20 Mhz */ {   65.0f,		/* SGI */   72.2f, },
		/* 40 Mhz */ {  135.0f,		/* SGI */  150.0f, },
	},

	/* MCS 42  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 43  */
	{	/* 20 Mhz */ {   91.0f,		/* SGI */  101.1f, },
		/* 40 Mhz */ {  189.0f,		/* SGI */  210.0f, },
	},

	/* MCS 44  */
	{	/* 20 Mhz */ {   91.0f,		/* SGI */  101.1f, },
		/* 40 Mhz */ {  189.0f,		/* SGI */  210.0f, },
	},

	/* MCS 45  */
	{	/* 20 Mhz */ {  104.0f,		/* SGI */  115.6f, },
		/* 40 Mhz */ {  216.0f,		/* SGI */  240.0f, },
	},

	/* MCS 46  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 47  */
	{	/* 20 Mhz */ {   97.5f,		/* SGI */  108.3f, },
		/* 40 Mhz */ {  202.5f,		/* SGI */  225.0f, },
	},

	/* MCS 48  */
	{	/* 20 Mhz */ {   97.5f,		/* SGI */  108.3f, },
		/* 40 Mhz */ {  202.5f,		/* SGI */  225.0f, },
	},

	/* MCS 49  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 50  */
	{	/* 20 Mhz */ {  136.5f,		/* SGI */  151.7f, },
		/* 40 Mhz */ {  283.5f,		/* SGI */  315.0f, },
	},

	/* MCS 51  */
	{	/* 20 Mhz */ {  136.5f,		/* SGI */  151.7f, },
		/* 40 Mhz */ {  283.5f,		/* SGI */  315.0f, },
	},

	/* MCS 52  */
	{	/* 20 Mhz */ {  156.0f,		/* SGI */  173.3f, },
		/* 40 Mhz */ {  324.0f,		/* SGI */  360.0f, },
	},

	/* MCS 53  */
	{	/* 20 Mhz */ {   65.0f,		/* SGI */   72.2f, },
		/* 40 Mhz */ {  135.0f,		/* SGI */  150.0f, },
	},

	/* MCS 54  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 55  */
	{	/* 20 Mhz */ {   91.0f,		/* SGI */  101.1f, },
		/* 40 Mhz */ {  189.0f,		/* SGI */  210.0f, },
	},

	/* MCS 56  */
	{	/* 20 Mhz */ {   78.0f,		/* SGI */   86.7f, },
		/* 40 Mhz */ {  162.0f,		/* SGI */  180.0f, },
	},

	/* MCS 57  */
	{	/* 20 Mhz */ {   91.0f,		/* SGI */  101.1f, },
		/* 40 Mhz */ {  189.0f,		/* SGI */  210.0f, },
	},

	/* MCS 58  */
	{	/* 20 Mhz */ {  104.0f,		/* SGI */  115.6f, },
		/* 40 Mhz */ {  216.0f,		/* SGI */  240.0f, },
	},

	/* MCS 59  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 60  */
	{	/* 20 Mhz */ {  104.0f,		/* SGI */  115.6f, },
		/* 40 Mhz */ {  216.0f,		/* SGI */  240.0f, },
	},

	/* MCS 61  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 62  */
	{	/* 20 Mhz */ {  130.0f,		/* SGI */  144.4f, },
		/* 40 Mhz */ {  270.0f,		/* SGI */  300.0f, },
	},

	/* MCS 63  */
	{	/* 20 Mhz */ {  130.0f,		/* SGI */  144.4f, },
		/* 40 Mhz */ {  270.0f,		/* SGI */  300.0f, },
	},

	/* MCS 64  */
	{	/* 20 Mhz */ {  143.0f,		/* SGI */  158.9f, },
		/* 40 Mhz */ {  297.0f,		/* SGI */  330.0f, },
	},

	/* MCS 65  */
	{	/* 20 Mhz */ {   97.5f,		/* SGI */  108.3f, },
		/* 40 Mhz */ {  202.5f,		/* SGI */  225.0f, },
	},

	/* MCS 66  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 67  */
	{	/* 20 Mhz */ {  136.5f,		/* SGI */  151.7f, },
		/* 40 Mhz */ {  283.5f,		/* SGI */  315.0f, },
	},

	/* MCS 68  */
	{	/* 20 Mhz */ {  117.0f,		/* SGI */  130.0f, },
		/* 40 Mhz */ {  243.0f,		/* SGI */  270.0f, },
	},

	/* MCS 69  */
	{	/* 20 Mhz */ {  136.5f,		/* SGI */  151.7f, },
		/* 40 Mhz */ {  283.5f,		/* SGI */  315.0f, },
	},

	/* MCS 70  */
	{	/* 20 Mhz */ {  156.0f,		/* SGI */  173.3f, },
		/* 40 Mhz */ {  324.0f,		/* SGI */  360.0f, },
	},

	/* MCS 71  */
	{	/* 20 Mhz */ {  175.5f,		/* SGI */  195.0f, },
		/* 40 Mhz */ {  364.5f,		/* SGI */  405.0f, },
	},

	/* MCS 72  */
	{	/* 20 Mhz */ {  156.0f,		/* SGI */  173.3f, },
		/* 40 Mhz */ {  324.0f,		/* SGI */  360.0f, },
	},

	/* MCS 73  */
	{	/* 20 Mhz */ {  175.5f,		/* SGI */  195.0f, },
		/* 40 Mhz */ {  364.5f,		/* SGI */  405.0f, },
	},

	/* MCS 74  */
	{	/* 20 Mhz */ {  195.0f,		/* SGI */  216.7f, },
		/* 40 Mhz */ {  405.0f,		/* SGI */  450.0f, },
	},

	/* MCS 75  */
	{	/* 20 Mhz */ {  195.0f,		/* SGI */  216.7f, },
		/* 40 Mhz */ {  405.0f,		/* SGI */  450.0f, },
	},

	/* MCS 76  */
	{	/* 20 Mhz */ {  214.5f,		/* SGI */  238.3f, },
		/* 40 Mhz */ {  445.5f,		/* SGI */  495.0f, },
	},
};

void ieee80211_parser::ieee80211_parse::print_header_type(){
	char buffer[100];
	switch (hdr.hdr_type){
    case MGMT_HDR:
        printf("MGMT_HDR\n");
        break;
    case CTRL_HDR:
        printf("CTRL_HDR type:");
		switch (hdr.hdr.ctrl_hdr.ctrl_hdr_type){
		case RTS_HDR:
			printf("RTS\n");
			break;
		case CTS_HDR:
			printf("CTS\n");
			break;
		case ACK_HDR:
			macaddr_ntoa(buffer, hdr.hdr.ctrl_hdr.hdr.ba.ra);
			printf("ACK %s\n", buffer);
			break;
		case PS_POLL_HDR:
			printf("PS_POLL\n");
			break;
		case CF_END_HDR:
			printf("CF_END\n");
			break;
		case CF_END_ACK_HDR:
			printf("CF_END_ACK\n");
			break;
		case BA_HDR:
			macaddr_ntoa(buffer, hdr.hdr.ctrl_hdr.hdr.ba.ra);
			printf("BA %s\n", buffer);
			break;
		case BAR_HDR:
			printf("BAR\n");
		default:
			break;
		}
        break;
    case DATA_HDR:
        printf("DATA_HDR\n");
        break;
    default:
        break;
    }
}

int  ieee80211_parser::ieee80211_parse::parse_elements( struct mgmt_body_t *mgmt_body, const uint8_t *p, int offset, uint32_t length){
    uint32_t elementlen;
	struct ssid_t ssid;
	struct challenge_t challenge;
	struct rates_t rates;
	struct ds_t ds;
	struct cf_t cf;
	struct tim_t tim;
	struct meshid_t meshid;

	/*
	 * We haven't seen any elements yet.
	 */
	mgmt_body->challenge_present = 0;
	mgmt_body->ssid_present = 0;
	mgmt_body->rates_present = 0;
	mgmt_body->ds_present = 0;
	mgmt_body->cf_present = 0;
	mgmt_body->tim_present = 0;
	mgmt_body->meshid_present = 0;

	while (length != 0) {

		elementlen = GET_U_1(p + offset + 1);


		switch (GET_U_1(p + offset)) {
		case E_SSID:
			ssid.length = elementlen;
			offset += 2;
			length -= 2;
			if (ssid.length != 0) {
				if (ssid.length > sizeof(ssid.ssid) - 1)
					return 0;
				memcpy(&ssid.ssid, p + offset, ssid.length);
				offset += ssid.length;
				length -= ssid.length;
			}
			ssid.ssid[ssid.length] = '\0';
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen an SSID IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->ssid_present) {
				mgmt_body->ssid = ssid;
				mgmt_body->ssid_present = 1;
			}
			break;
		case E_CHALLENGE:
			challenge.length = elementlen;
			offset += 2;
			length -= 2;
			if (challenge.length != 0) {
				if (challenge.length >
				    sizeof(challenge.text) - 1)
					return 0;
				memcpy(&challenge.text, p + offset,
				    challenge.length);
				offset += challenge.length;
				length -= challenge.length;
			}
			challenge.text[challenge.length] = '\0';
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a challenge IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->challenge_present) {
				mgmt_body->challenge = challenge;
				mgmt_body->challenge_present = 1;
			}
			break;
		case E_RATES:
			rates.length = elementlen;
			offset += 2;
			length -= 2;
			if (rates.length != 0) {
				if (rates.length > sizeof(rates.rate))
					return 0;
				memcpy(&rates.rate, p + offset, rates.length);
				offset += rates.length;
				length -= rates.length;
			}
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a rates IE,
			 * copy this one if it's not zero-length,
			 * otherwise ignore this one, so we later
			 * report the first one we saw.
			 *
			 * We ignore zero-length rates IEs as some
			 * devices seem to put a zero-length rates
			 * IE, followed by an SSID IE, followed by
			 * a non-zero-length rates IE into frames,
			 * even though IEEE Std 802.11-2007 doesn't
			 * seem to indicate that a zero-length rates
			 * IE is valid.
			 */
			if (!mgmt_body->rates_present && rates.length != 0) {
				mgmt_body->rates = rates;
				mgmt_body->rates_present = 1;
			}
			break;
		case E_DS:
			ds.length = elementlen;
			offset += 2;
			length -= 2;
			if (ds.length != 1) {
				offset += ds.length;
				length -= ds.length;
				break;
			}
			ds.channel = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a DS IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->ds_present) {
				mgmt_body->ds = ds;
				mgmt_body->ds_present = 1;
			}
			break;
		case E_CF:
			cf.length = elementlen;
			offset += 2;
			length -= 2;
			if (cf.length != 6) {
				offset += cf.length;
				length -= cf.length;
				break;
			}
			cf.count = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			cf.period = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			cf.max_duration = GET_LE_U_2(p + offset);
			offset += 2;
			length -= 2;
			cf.dur_remaining = GET_LE_U_2(p + offset);
			offset += 2;
			length -= 2;
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a CF IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->cf_present) {
				mgmt_body->cf = cf;
				mgmt_body->cf_present = 1;
			}
			break;
		case E_TIM:
			tim.length = elementlen;
			offset += 2;
			length -= 2;
			if (tim.length <= 3U) {
				offset += tim.length;
				length -= tim.length;
				break;
			}
			if (tim.length - 3U > sizeof(tim.bitmap))
				return 0;
			tim.count = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			tim.period = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			tim.bitmap_control = GET_U_1(p + offset);
			offset += 1;
			length -= 1;
			memcpy(tim.bitmap, p + offset, tim.length - 3);
			offset += tim.length - 3;
			length -= tim.length - 3;
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a TIM IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->tim_present) {
				mgmt_body->tim = tim;
				mgmt_body->tim_present = 1;
			}
			break;
		case E_MESHID:
			meshid.length = elementlen;
			offset += 2;
			length -= 2;
			if (meshid.length != 0) {
				if (meshid.length > sizeof(meshid.meshid) - 1)
					return 0;
				memcpy(&meshid.meshid, p + offset, meshid.length);
				offset += meshid.length;
				length -= meshid.length;
			}
			meshid.meshid[meshid.length] = '\0';
			/*
			 * Present and not truncated.
			 *
			 * If we haven't already seen a MESHID IE,
			 * copy this one, otherwise ignore this one,
			 * so we later report the first one we saw.
			 */
			if (!mgmt_body->meshid_present) {
				mgmt_body->meshid = meshid;
				mgmt_body->meshid_present = 1;
			}
			break;
		default:
			offset += 2 + elementlen;
			length -= 2 + elementlen;
			break;
		}
	}

	/* No problems found. */
	return 1;
}

int  ieee80211_parser::ieee80211_parse::handle_beacon(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	memcpy(&body.mgmt_body.timestamp, p, IEEE802_11_TSTAMP_LEN);
	offset += IEEE802_11_TSTAMP_LEN;
	length -= IEEE802_11_TSTAMP_LEN;
	body.mgmt_body.beacon_interval = GET_LE_U_2(p + offset);
	offset += IEEE802_11_BCNINT_LEN;
	length -= IEEE802_11_BCNINT_LEN;
	body.mgmt_body.capability_info = GET_LE_U_2(p + offset);
	offset += IEEE802_11_CAPINFO_LEN;
	length -= IEEE802_11_CAPINFO_LEN;

	ret = parse_elements(&body.mgmt_body, p, offset, length);

	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_assoc_request(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body)); 

	body.mgmt_body.capability_info = GET_LE_U_2(p);
	offset += IEEE802_11_CAPINFO_LEN;
	length -= IEEE802_11_CAPINFO_LEN;
	body.mgmt_body.listen_interval = GET_LE_U_2(p + offset);
	offset += IEEE802_11_LISTENINT_LEN;
	length -= IEEE802_11_LISTENINT_LEN;

	ret = parse_elements(&body.mgmt_body, p, offset, length);

	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_assoc_response(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	body.mgmt_body.capability_info = GET_LE_U_2(p);
	offset += IEEE802_11_CAPINFO_LEN;
	length -= IEEE802_11_CAPINFO_LEN;
	body.mgmt_body.status_code = GET_LE_U_2(p + offset);
	offset += IEEE802_11_STATUS_LEN;
	length -= IEEE802_11_STATUS_LEN;
	body.mgmt_body.aid = GET_LE_U_2(p + offset);
	offset += IEEE802_11_AID_LEN;
	length -= IEEE802_11_AID_LEN;

	ret = parse_elements(&body.mgmt_body, p, offset, length);


	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_reassoc_request(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));
	body.mgmt_body.capability_info = GET_LE_U_2(p);
	offset += IEEE802_11_CAPINFO_LEN;
	length -= IEEE802_11_CAPINFO_LEN;
	body.mgmt_body.listen_interval = GET_LE_U_2(p + offset);
	offset += IEEE802_11_LISTENINT_LEN;
	length -= IEEE802_11_LISTENINT_LEN;
	memcpy(&body.mgmt_body.ap, p+offset, IEEE802_11_AP_LEN);
	offset += IEEE802_11_AP_LEN;
	length -= IEEE802_11_AP_LEN;

	ret = parse_elements(&body.mgmt_body, p, offset, length);

	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_reassoc_response(const uint8_t *p, uint32_t length){
	/* Same as a Association Response */
	return handle_assoc_response(p, length);
}

int  ieee80211_parser::ieee80211_parse::handle_probe_request(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	ret = parse_elements(&body.mgmt_body, p, offset, length);

	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_probe_response(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	memcpy(&body.mgmt_body.timestamp, p, IEEE802_11_TSTAMP_LEN);
	offset += IEEE802_11_TSTAMP_LEN;
	length -= IEEE802_11_TSTAMP_LEN;
	body.mgmt_body.beacon_interval = GET_LE_U_2(p + offset);
	offset += IEEE802_11_BCNINT_LEN;
	length -= IEEE802_11_BCNINT_LEN;
	body.mgmt_body.capability_info = GET_LE_U_2(p + offset);
	offset += IEEE802_11_CAPINFO_LEN;
	length -= IEEE802_11_CAPINFO_LEN;

	ret = parse_elements(&body.mgmt_body, p, offset, length);
	return ret;  
}

int  ieee80211_parser::ieee80211_parse::handle_atim(void){
	/* the frame body for ATIM is null. */
	return 1;
}

int  ieee80211_parser::ieee80211_parse::handle_disassoc(const uint8_t *p, uint32_t length){
	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));
 
	body.mgmt_body.reason_code = GET_LE_U_2(p);

	return 1;
}

int  ieee80211_parser::ieee80211_parse::handle_auth(const uint8_t *p, uint32_t length){
	int offset = 0;
	int ret;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	body.mgmt_body.auth_alg = GET_LE_U_2(p);
	offset += 2;
	length -= 2;
	body.mgmt_body.auth_trans_seq_num = GET_LE_U_2(p + offset);
	offset += 2;
	length -= 2;
	body.mgmt_body.status_code = GET_LE_U_2(p + offset);
	offset += 2;
	length -= 2;

	ret = parse_elements(&body.mgmt_body, p, offset, length);

	return ret;
}

int  ieee80211_parser::ieee80211_parse::handle_deauth(const uint8_t *src, const uint8_t *p, uint32_t length){
	const char *reason = NULL;

	memset(&body.mgmt_body, 0, sizeof(body.mgmt_body));

	body.mgmt_body.reason_code = GET_LE_U_2(p);

	return 1;
}

int  ieee80211_parser::ieee80211_parse::handle_action(const uint8_t *src, const uint8_t *p, uint32_t length){

	body.mgmt_body.category = GET_U_1(p);
	body.mgmt_body.action = GET_U_1(p + 1);

	return 1;
}

int  ieee80211_parser::ieee80211_parse::wep_parse(const uint8_t *p){

	iv = GET_LE_U_4(p);

	return 1;
}

int  ieee80211_parser::ieee80211_parse::mgmt_body_parse(uint16_t fc, const uint8_t *src, const uint8_t *p, uint32_t length){
	if (FC_PROTECTED(fc))
		return wep_parse(p);
	switch (FC_SUBTYPE(fc)) {
	case ST_ASSOC_REQUEST:
		return handle_assoc_request(p, length);
	case ST_ASSOC_RESPONSE:
		return handle_assoc_response(p, length);
	case ST_REASSOC_REQUEST:
		return handle_reassoc_request(p, length);
	case ST_REASSOC_RESPONSE:
		return handle_reassoc_response(p, length);
	case ST_PROBE_REQUEST:
		return handle_probe_request(p, length);
	case ST_PROBE_RESPONSE:
		return handle_probe_response(p, length);
	case ST_BEACON:
		return handle_beacon(p, length);
	case ST_ATIM:
		return handle_atim();
	case ST_DISASSOC:
		return handle_disassoc(p, length);
	case ST_AUTH:
		return handle_auth(p, length);
	case ST_DEAUTH:
		return handle_deauth(src, p, length);
	case ST_ACTION:
		return handle_action(src, p, length);
	default:
		return 1;
	}
}

int  ieee80211_parser::ieee80211_parse::ctrl_body_parse(uint16_t fc, const uint8_t *p){
	switch (FC_SUBTYPE(fc)) {
	case CTRL_CONTROL_WRAPPER:
		/* XXX - requires special handling */
		break;
	case CTRL_BAR:
		memcpy(hdr.hdr.ctrl_hdr.hdr.bar.ra, ((const struct ctrl_bar_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.bar.ra));
		memcpy(hdr.hdr.ctrl_hdr.hdr.bar.ta, ((const struct ctrl_bar_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.bar.ta));
		hdr.hdr.ctrl_hdr.hdr.bar.ctl = GET_LE_U_2(&((const struct ctrl_bar_hdr_t *)p)->ctl),
		hdr.hdr.ctrl_hdr.hdr.bar.seq = GET_LE_U_2(&((const struct ctrl_bar_hdr_t *)p)->seq);
		break;
	case CTRL_BA:
		memcpy(hdr.hdr.ctrl_hdr.hdr.ba.ra, ((const struct ctrl_ba_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.ba.ra));
		hdr.hdr.ctrl_hdr.hdr.ba.ssc = ((const struct ctrl_ba_hdr_t *)p)->ssc; 
		memcpy(&hdr.hdr.ctrl_hdr.hdr.ba.bitmap, p+CTRL_BA_HDRLEN+4, sizeof(uint64_t));
		hdr.hdr.ctrl_hdr.hdr.ba.bitmap = __bswap_64(hdr.hdr.ctrl_hdr.hdr.ba.bitmap);
		break;
	case CTRL_PS_POLL:
		hdr.hdr.ctrl_hdr.hdr.ps_poll.aid = GET_LE_U_2(&((const struct ctrl_ps_poll_hdr_t *)p)->aid);
		break;
	case CTRL_RTS:
		memcpy(hdr.hdr.ctrl_hdr.hdr.rts.ta, ((const struct ctrl_rts_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.rts.ta));
		break;
	case CTRL_CTS:
		memcpy(hdr.hdr.ctrl_hdr.hdr.cts.ra, ((const struct ctrl_cts_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.cts.ra));
		break;
	case CTRL_ACK:
		memcpy(hdr.hdr.ctrl_hdr.hdr.ack.ra, ((const struct ctrl_ack_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.ack.ra));
		hdr.hdr.ctrl_hdr.hdr.ack.ssc = *(uint16_t*)(p+22);
		break;
	case CTRL_CF_END:
		memcpy(hdr.hdr.ctrl_hdr.hdr.end.ra, ((const struct ctrl_end_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.end.ra));
		break;
	case CTRL_END_ACK:
		memcpy(hdr.hdr.ctrl_hdr.hdr.end_ack.ra, ((const struct ctrl_end_ack_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.end_ack.ra));
		break;
	}
	return 1;
}

/**
 * Data Frame - Address field contents
 *
 *  To Ds  | From DS | Addr 1 | Addr 2 | Addr 3 | Addr 4
 *    0    |  0      |  DA    | SA     | BSSID  | n/a
 *    0    |  1      |  DA    | BSSID  | SA     | n/a
 *    1    |  0      |  BSSID | SA     | DA     | n/a
 *    1    |  1      |  RA    | TA     | DA     | SA
 */

/**
 * @brief  to get source and destination MAC addresses for a data frame.
 */
void  ieee80211_parser::ieee80211_parse::get_data_src_dst_mac(uint16_t fc, const uint8_t *p, const uint8_t **srcp, const uint8_t **dstp){
#define ADDR1  (p + 4)
#define ADDR2  (p + 10)
#define ADDR3  (p + 16)
#define ADDR4  (p + 24)

	if (!FC_TO_DS(fc)) {
		if (!FC_FROM_DS(fc)) {
			/* not To DS and not From DS */
			*srcp = ADDR2;
			*dstp = ADDR1;
		} else {
			/* not To DS and From DS */
			*srcp = ADDR3;
			*dstp = ADDR1;
		}
	} else {
		if (!FC_FROM_DS(fc)) {
			/* To DS and not From DS */
			*srcp = ADDR2;
			*dstp = ADDR3;
		} else {
			/* To DS and From DS */
			*srcp = ADDR4;
			*dstp = ADDR3;
		}
	}

#undef ADDR1
#undef ADDR2
#undef ADDR3
#undef ADDR4
}

void  ieee80211_parser::ieee80211_parse::get_mgmt_src_dst_mac(const uint8_t *p, const uint8_t **srcp, const uint8_t **dstp){
	struct mgmt_header_t *hp = (struct mgmt_header_t *) p;

	if (srcp != NULL)
		*srcp = hp->sa;
	if (dstp != NULL)
		*dstp = hp->da;
}

void  ieee80211_parser::ieee80211_parse::data_header_parse(uint16_t fc, const uint8_t *p){
	uint32_t subtype = FC_SUBTYPE(fc);

#define ADDR1  (p + 4)
#define ADDR2  (p + 10)
#define ADDR3  (p + 16)
#define ADDR4  (p + 24)

	if (!FC_TO_DS(fc) && !FC_FROM_DS(fc)) {
		memcpy(hdr.hdr.data_hdr.da, ADDR1, sizeof(hdr.hdr.data_hdr.da));
		memcpy(hdr.hdr.data_hdr.sa, ADDR2, sizeof(hdr.hdr.data_hdr.sa));
		memcpy(hdr.hdr.data_hdr.bssid, ADDR3, sizeof(hdr.hdr.data_hdr.bssid));
	} 
	else if (!FC_TO_DS(fc) && FC_FROM_DS(fc)) {
		memcpy(hdr.hdr.data_hdr.da, ADDR1, sizeof(hdr.hdr.data_hdr.da));
		memcpy(hdr.hdr.data_hdr.sa, ADDR3, sizeof(hdr.hdr.data_hdr.sa));
		memcpy(hdr.hdr.data_hdr.bssid, ADDR2, sizeof(hdr.hdr.data_hdr.bssid));
	} 
	else if (FC_TO_DS(fc) && !FC_FROM_DS(fc)) {
		memcpy(hdr.hdr.data_hdr.da, ADDR3, sizeof(hdr.hdr.data_hdr.da));
		memcpy(hdr.hdr.data_hdr.sa, ADDR2, sizeof(hdr.hdr.data_hdr.sa));
		memcpy(hdr.hdr.data_hdr.bssid, ADDR1, sizeof(hdr.hdr.data_hdr.bssid));
	} 
	else if (FC_TO_DS(fc) && FC_FROM_DS(fc)) {
		memcpy(hdr.hdr.data_hdr.da, ADDR3, sizeof(hdr.hdr.data_hdr.da));
		memcpy(hdr.hdr.data_hdr.sa, ADDR4, sizeof(hdr.hdr.data_hdr.sa));
		memcpy(hdr.hdr.data_hdr.ra, ADDR1, sizeof(hdr.hdr.data_hdr.ra));
		memcpy(hdr.hdr.data_hdr.ta, ADDR2, sizeof(hdr.hdr.data_hdr.ta));
	}
	hdr.hdr.data_hdr.ssc = *(uint16_t*)(p+22);

#undef ADDR1
#undef ADDR2
#undef ADDR3
#undef ADDR4
}

void  ieee80211_parser::ieee80211_parse::mgmt_header_parse(const uint8_t *p){
	const struct mgmt_header_t *hp = (const struct mgmt_header_t *) p;

	memcpy(hdr.hdr.mgmt_hdr.bssid, hp->bssid, sizeof(hdr.hdr.mgmt_hdr.bssid));
	memcpy(hdr.hdr.mgmt_hdr.da, hp->da, sizeof(hdr.hdr.mgmt_hdr.da));
	memcpy(hdr.hdr.mgmt_hdr.sa, hp->sa, sizeof(hdr.hdr.mgmt_hdr.sa));

}

void  ieee80211_parser::ieee80211_parse::ctrl_header_parse(uint16_t fc, const uint8_t *p){
	switch (FC_SUBTYPE(fc)) {
	case CTRL_CONTROL_WRAPPER:
		/* XXX - requires special handling */
		break;
	case CTRL_BAR:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = BAR_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.bar.ra, ((const struct ctrl_bar_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.bar.ra));
		memcpy(hdr.hdr.ctrl_hdr.hdr.bar.ta, ((const struct ctrl_bar_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.bar.ta));
		hdr.hdr.ctrl_hdr.hdr.bar.ctl = GET_LE_U_2(&((const struct ctrl_bar_hdr_t *)p)->ctl),
		hdr.hdr.ctrl_hdr.hdr.bar.seq = GET_LE_U_2(&((const struct ctrl_bar_hdr_t *)p)->seq);
		break;
	case CTRL_BA:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = BA_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.ba.ra, ((const struct ctrl_ba_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.ba.ra));
		memcpy(hdr.hdr.ctrl_hdr.hdr.ba.ta, ((const struct ctrl_ba_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.ba.ta));
		break;
	case CTRL_PS_POLL:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = PS_POLL_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.ps_poll.bssid, ((const struct ctrl_ps_poll_hdr_t *)p)->bssid, sizeof(hdr.hdr.ctrl_hdr.hdr.ps_poll.bssid));
		memcpy(hdr.hdr.ctrl_hdr.hdr.ps_poll.ta, ((const struct ctrl_ps_poll_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.ps_poll.ta));
		break;
	case CTRL_RTS:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = RTS_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.rts.ta, ((const struct ctrl_rts_hdr_t *)p)->ta, sizeof(hdr.hdr.ctrl_hdr.hdr.rts.ta));
		memcpy(hdr.hdr.ctrl_hdr.hdr.rts.ra, ((const struct ctrl_rts_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.rts.ra));
		break;
	case CTRL_CTS:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = CTS_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.cts.ra, ((const struct ctrl_cts_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.cts.ra));
		break;
	case CTRL_ACK:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = ACK_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.ack.ra, ((const struct ctrl_ack_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.ack.ra));
		break;
	case CTRL_CF_END:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = CF_END_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.end.ra, ((const struct ctrl_end_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.end.ra));
		memcpy(hdr.hdr.ctrl_hdr.hdr.end.bssid, ((const struct ctrl_end_hdr_t *)p)->bssid, sizeof(hdr.hdr.ctrl_hdr.hdr.end.bssid));
		break;
	case CTRL_END_ACK:
		hdr.hdr.ctrl_hdr.ctrl_hdr_type = CF_END_ACK_HDR;
		memcpy(hdr.hdr.ctrl_hdr.hdr.end_ack.ra, ((const struct ctrl_end_ack_hdr_t *)p)->ra, sizeof(hdr.hdr.ctrl_hdr.hdr.end_ack.ra));
		memcpy(hdr.hdr.ctrl_hdr.hdr.end_ack.bssid, ((const struct ctrl_end_ack_hdr_t *)p)->bssid, sizeof(hdr.hdr.ctrl_hdr.hdr.end_ack.bssid));
		break;
	default:
		break;
	}
}

int  ieee80211_parser::ieee80211_parse::extract_header_length(uint16_t fc)
{
	int len;

	switch (FC_TYPE(fc)) {
	case T_MGMT:
		return MGMT_HDRLEN;
	case T_CTRL:
		switch (FC_SUBTYPE(fc)) {
		case CTRL_CONTROL_WRAPPER:
			return CTRL_CONTROL_WRAPPER_HDRLEN;
		case CTRL_BAR:
			return CTRL_BAR_HDRLEN;
		case CTRL_BA:
			return CTRL_BA_HDRLEN;
		case CTRL_PS_POLL:
			return CTRL_PS_POLL_HDRLEN;
		case CTRL_RTS:
			return CTRL_RTS_HDRLEN;
		case CTRL_CTS:
			return CTRL_CTS_HDRLEN;
		case CTRL_ACK:
			return CTRL_ACK_HDRLEN;
		case CTRL_CF_END:
			return CTRL_END_HDRLEN;
		case CTRL_END_ACK:
			return CTRL_END_ACK_HDRLEN;
		default:
			fprintf(stderr, "unknown 802.11 ctrl frame subtype (%u)\n", FC_SUBTYPE(fc));
			return 0;
		}
	case T_DATA:
		len = (FC_TO_DS(fc) && FC_FROM_DS(fc)) ? 30 : 24;
		if (DATA_FRAME_IS_QOS(FC_SUBTYPE(fc)))
			len += 2;
		return len;
	default:
		fprintf(stderr, "unknown 802.11 frame type (%u)\n", FC_TYPE(fc));
		return 0;
	}
}

int  ieee80211_parser::ieee80211_parse::extract_mesh_header_length(const uint8_t *p){
	return (GET_U_1(p) &~ 3) ? 0 : 6*(1 + (GET_U_1(p) & 3));
}

/**
 * @brief Parse the 802.11 MAC header.
 */
void  ieee80211_parser::ieee80211_parse::ieee_802_11_hdr_parse(uint16_t fc, const uint8_t *p, uint32_t hdrlen, uint32_t meshdrlen){
	switch (FC_TYPE(fc)) {
	case T_MGMT:
		hdr.hdr_type = MGMT_HDR;
		mgmt_header_parse(p);
		break;
	case T_CTRL:
		hdr.hdr_type = CTRL_HDR;
		ctrl_header_parse(fc, p);
		break;
	case T_DATA:
		hdr.hdr_type = DATA_HDR;
		data_header_parse(fc, p);
		break;
	default:
		fprintf(stderr, "unkonw frame, detect in header parse\n");
		break;
	}
}

uint32_t  ieee80211_parser::ieee80211_parse::ieee802_11_parse(const uint8_t *p, uint32_t length, uint32_t orig_caplen, int pad, uint32_t fcslen){
	uint16_t fc;
	uint32_t caplen, hdrlen, meshdrlen;
	int llc_hdrlen = 0;

	caplen = orig_caplen;
	/* Remove FCS, if present */
	if (length < fcslen) {
		return caplen;
	}
	length -= fcslen;
	if (caplen > length) {
		/* Amount of FCS in actual packet data, if any */
		fcslen = caplen - length;
		caplen -= fcslen;
	}

	if (caplen < IEEE802_11_FC_LEN) {
		return orig_caplen;
	}

	fc = GET_LE_U_2(p);
	parse_fc = fc;
	hdrlen = extract_header_length(fc);
	if (hdrlen == 0) {
		/* Unknown frame type or control frame subtype; quit. */
		printf("unkown frame, detect in fc\n");
		return (0);
	}
	if (pad)
		hdrlen = roundup2(hdrlen, 4);
	if (FC_TYPE(fc) == T_DATA &&
	    DATA_FRAME_IS_QOS(FC_SUBTYPE(fc))) {

		meshdrlen = extract_mesh_header_length(p + hdrlen);
		hdrlen += meshdrlen;
	} else
		meshdrlen = 0;

	if (caplen < hdrlen) {
		return hdrlen;
	}
	ieee_802_11_hdr_parse(fc, p, hdrlen, meshdrlen);

	/*
	 * Go past the 802.11 header.
	 */
	length -= hdrlen;
	caplen -= hdrlen;
	p += hdrlen;

	switch (FC_TYPE(fc)) {
	case T_MGMT:
		get_mgmt_src_dst_mac(p - hdrlen, &mac_src, &mac_dst);
		if (!mgmt_body_parse(fc, mac_src, p, length)) {
			return hdrlen;
		}
		break;
	case T_CTRL:
		if (!ctrl_body_parse(fc, p - hdrlen)) {
			return hdrlen;
		}
		break;
	case T_DATA:
		if (DATA_FRAME_IS_NULL(FC_SUBTYPE(fc)))
			return hdrlen;	/* no-data frame */
		/* There may be a problem w/ AP not having this bit set */
		if (FC_PROTECTED(fc)) {
			if (!wep_parse(p)) {
				return hdrlen;
			}
		} 
		else {
			get_data_src_dst_mac(fc, p - hdrlen, &mac_src, &mac_dst);
			/* we don't care about llc*/
			// llc_hdrlen = llc_parse(ndo, p, length, caplen, &src, &dst);
			// if (llc_hdrlen < 0) {
			// 	/*
			// 	 * Some kinds of LLC packet we cannot
			// 	 * handle intelligently
			// 	 */
			// 	llc_hdrlen = -llc_hdrlen;
			// }
			// hdrlen += llc_hdrlen;
		}
		break;
	default:
		/* We shouldn't get here - we should already have quit */
		fprintf(stderr, "detect a error frame\n");
		break;
	}

	return hdrlen;
}

/**
 * @brief This is the top level routine of the printer.  'p' points
 * to the 802.11 header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 **/
void  ieee80211_parser::ieee80211_parse::ieee802_11_if_parse(const struct pcap_pkthdr *h, const uint8_t *p){
	ieee802_11_parse(p, h->len, h->caplen, 0, 0);
}

/**
 * the definitions below are used to handle radio stamp, we can get it from nl80211
 * 
 * ***/

void  ieee80211_parser::ieee80211_parse::parse_chaninfo(uint16_t freq, uint32_t flags, uint32_t presentflags){
	radio.freq = freq;
	int pos = 0;
	if (presentflags & (1 << IEEE80211_RADIOTAP_MCS)) {
		/*
		 * We have the MCS field, so this is 11n, regardless
		 * of what the channel flags say.
		 */
		pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11n");
	} else {
		if (IS_CHAN_FHSS(flags))
			pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " FHSS");
		if (IS_CHAN_A(flags)) {
			if (flags & IEEE80211_CHAN_HALF){
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11a/10Mhz");
			}
			else if (flags & IEEE80211_CHAN_QUARTER){
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11a/5Mhz");
			}
			else {
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11a");
			}
		}
		if (IS_CHAN_ANYG(flags)) {
			if (flags & IEEE80211_CHAN_HALF){
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11g/10Mhz");
			}
			else if (flags & IEEE80211_CHAN_QUARTER){
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11g/5Mhz");
			}
			else{
				pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11g");
			}
		} 
		else if (IS_CHAN_B(flags)){
			pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " 11b");
		}
		if (flags & IEEE80211_CHAN_TURBO){
			pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " Turbo");
		}
	}
	/*
	 * These apply to 11n.
	 */
	if (flags & IEEE80211_CHAN_HT20){
		if (flags & IEEE80211_CHAN_TURBO){
			pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " ht/20");
		}
	}
	else if (flags & IEEE80211_CHAN_HT40D){
		pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " ht/40-");
	}
	else if (flags & IEEE80211_CHAN_HT40U){
		pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " ht/40+");
	}
	pos += snprintf(radio.mcs_info+pos, MCS_INFO_LEN-pos, " ");
}


