#include <libnetfilter_queue/libnetfilter_queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/netfilter.h>

struct decoded_packet {
  uint8_t version; // 4 = ipv4, 6 = ipv6
  uint8_t ip_protocol; // 6 = tcp, 17 = udp
};

void decode_l4_pdu(unsigned char * l4buf, int l4len,
    struct decoded_packet * decpkt) {
}

void decode_ipv4_packet(unsigned char * buf, int len,
    struct decoded_packet * decpkt) {
  decpkt->ip_protocol = buf[9];
  unsigned char * l4buf = buf + (buf[0]&15)*4; // buf + ihl*4
  decode_l4_pdu(l4buf, len - (l4buf - buf), decpkt);
}

void decode_ipv6_packet(unsigned char * buf, int len,
    struct decoded_packet * decpkt) {
  const unsigned char * limit = buf + len;
  unsigned char * l4buf = buf + 40;
  decpkt->ip_protocol = buf[6];
  do {
    switch (decpkt->ip_protocol) {
      case 59: // no next header
	return;
      case 0: // hop by hop options
      case 60: // destination options
      case 43: // routing options
      case 44: // fragment extension
      case 135: // mobility header
	l4buf += 8 + l4buf[1]*8; // += 8 bytes + payload len * 4 bytes
	break;
      case 51: // authentication header
	l4buf += 8 + l4buf[1]*4; // += 8 bytes + payload len * 4 bytes
	break;
      default:
	decode_l4_pdu(l4buf, limit - l4buf, decpkt);
	return;
    }
  } while (l4buf < limit);
}

void decode_packet(unsigned char * buf, int len,
    struct decoded_packet * decpkt) {
  decpkt->version = (buf[0]>>4) & 15;
  switch (decpkt->version) {
    case 4:
      decode_ipv4_packet(buf, len, decpkt);
      break;
    case 6:
      decode_ipv6_packet(buf, len, decpkt);
  }
}

int callback(struct nfq_q_handle * nfqqh, struct nfgenmsg * nfmsg,
    struct nfq_data * nfad, void * data) {
  int mark = 42;
  struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfad);
  uint32_t id = ntohl(ph->packet_id);
  unsigned char * buf;
  int data_len = nfq_get_payload(nfad, (char**)&buf);
  struct decoded_packet decpkt;
  decode_packet(buf, data_len, &decpkt);
  printf("data_len = %d\n", data_len);
  printf("  version = %d\n", decpkt.version);
  printf("  protocol = %d (tcp=6,udp=17)\n", decpkt.ip_protocol);
  nfq_set_verdict_mark(nfqqh, id, NF_ACCEPT, htonl(mark), data_len, buf);
  return 0;
}

void run_example(void) {
  struct nfq_handle * nfqh;
  struct nfq_q_handle * nfqqh;
  if (!(nfqh = nfq_open())) {
    fprintf(stderr, "error during nfq_open()\n");
    exit(1);
  }
  if (nfq_unbind_pf(nfqh, AF_INET) < 0) {
    fprintf(stderr, "error during nfq_unbind_pf(..., AF_INET)\n");
    exit(1);
  }
  if (nfq_unbind_pf(nfqh, AF_INET6) < 0) {
    fprintf(stderr, "error during nfq_unbind_pf(..., AF_INET6)\n");
    exit(1);
  }

  if (nfq_bind_pf(nfqh, AF_INET) < 0) {
    fprintf(stderr, "error during nfq_bind_pf()\n");
    exit(1);
  }
  if (nfq_bind_pf(nfqh, AF_INET6) < 0) {
    fprintf(stderr, "error during nfq_bind_pf()\n");
    exit(1);
  }
  if (!(nfqqh = nfq_create_queue(nfqh, 42, &callback, NULL))) {
    fprintf(stderr, "error during nfq_create_queue()\n");
    exit(1);
  }
  if (nfq_set_mode(nfqqh, NFQNL_COPY_PACKET, 0xffff) < 0) {
    fprintf(stderr, "can't set packet_copy mode\n");
    exit(1);
  }

  int fd = nfq_fd(nfqh);
  char buff[5000];
  int res;
  while ((res = recv(fd, buff, sizeof buff, 0)) >= 0) {
    nfq_handle_packet(nfqh, buff, res);
  }
  nfq_destroy_queue(nfqqh);
}

int main(void) {
  run_example();
  return 0;
}
