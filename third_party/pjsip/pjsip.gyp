# Copyright (c) 2014 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../build/win_precompile.gypi',
  ],
  'variables': {
    'pjsip_source%': "source",
  },
  'target_defaults': {
    'include_dirs': [
      './overrides',
      './<(pjsip_source)/pjlib/include',
      './<(pjsip_source)/pjlib-util/include',
      './<(pjsip_source)/pjmedia/include',
      './<(pjsip_source)/pjnath/include',
      './<(pjsip_source)/pjsip/include',
    ],
    'dependencies': [
      '<(DEPTH)/third_party/speex/speex.gyp:libspeex',
      '<(DEPTH)/third_party/openssl/openssl.gyp:openssl',
    ],
    'link_settings': {
      'msvs_settings': {
        'VCLinkerTool': {
          'IgnoreDefaultLibraryNames': ['libeay32.lib', 'ssleay32.lib'],
        },
      }
    },
    'direct_dependent_settings': {
      'include_dirs': [
        './overrides',
        './<(pjsip_source)/pjlib/include',
        './<(pjsip_source)/pjlib-util/include',
        './<(pjsip_source)/pjmedia/include',
        './<(pjsip_source)/pjnath/include',
        './<(pjsip_source)/pjsip/include',
      ],
    },
    'export_dependent_settings': [
      '<(DEPTH)/third_party/speex/speex.gyp:libspeex',
      '<(DEPTH)/third_party/openssl/openssl.gyp:openssl',
    ],
    'conditions': [
      ['OS=="win"', {
        'msvs_disabled_warnings': [ 4005, 4267 ],
      }],
      ['os_posix == 1', {
        'defines': [
          'PJ_HAS_NETINET_TCP_H=1',
        ],
      }],
    ],
    'defines': [
      'PJMEDIA_HAS_SRTP=0',
      'PJMEDIA_RESAMPLE_IMP=PJMEDIA_RESAMPLE_NONE',
      'PJ_HAS_SSL_SOCK=1',
    ],
  },
  'targets': [
    {
      'target_name': 'pjlib',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjlib/src/pj/activesock.c',
        '<(pjsip_source)/pjlib/src/pj/array.c',
        '<(pjsip_source)/pjlib/src/pj/config.c',
        '<(pjsip_source)/pjlib/src/pj/ctype.c',
        '<(pjsip_source)/pjlib/src/pj/errno.c',
        '<(pjsip_source)/pjlib/src/pj/fifobuf.c',
        '<(pjsip_source)/pjlib/src/pj/guid.c',
        '<(pjsip_source)/pjlib/src/pj/list.c',
        '<(pjsip_source)/pjlib/src/pj/lock.c',
        '<(pjsip_source)/pjlib/src/pj/log.c',
        '<(pjsip_source)/pjlib/src/pj/os_info.c',
        '<(pjsip_source)/pjlib/src/pj/os_timestamp_common.c',
        '<(pjsip_source)/pjlib/src/pj/pool.c',
        '<(pjsip_source)/pjlib/src/pj/pool_buf.c',
        '<(pjsip_source)/pjlib/src/pj/pool_caching.c',
        '<(pjsip_source)/pjlib/src/pj/rand.c',
        '<(pjsip_source)/pjlib/src/pj/rbtree.c',
        '<(pjsip_source)/pjlib/src/pj/ssl_sock_common.c',
        '<(pjsip_source)/pjlib/src/pj/ssl_sock_dump.c',
        'overrides/src/pj/ssl_sock_ossl.c',
        '<(pjsip_source)/pjlib/src/pj/sock_common.c',
        '<(pjsip_source)/pjlib/src/pj/sock_qos_common.c',
        '<(pjsip_source)/pjlib/src/pj/sock_qos_bsd.c',
        '<(pjsip_source)/pjlib/src/pj/sock_qos_dummy.c',
        '<(pjsip_source)/pjlib/src/pj/string.c',
        '<(pjsip_source)/pjlib/src/pj/types.c',
        '<(pjsip_source)/pjlib/src/pj/hash.c',
        '<(pjsip_source)/pjlib/src/pj/except.c',
        '<(pjsip_source)/pjlib/src/pj/ioqueue_common_abs.h',
        '<(pjsip_source)/pjlib/src/pj/pool_dbg.c',
        '<(pjsip_source)/pjlib/src/pj/pool_policy_malloc.c',
        '<(pjsip_source)/pjlib/src/pj/sock_bsd.c',
        '<(pjsip_source)/pjlib/src/pj/sock_select.c',
        '<(pjsip_source)/pjlib/src/pj/symbols.c',
        '<(pjsip_source)/pjlib/src/pj/timer.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            '<(pjsip_source)/pjlib/src/pj/file_access_win32.c',
            '<(pjsip_source)/pjlib/src/pj/file_io_win32.c',
            '<(pjsip_source)/pjlib/src/pj/os_core_win32.c',
            '<(pjsip_source)/pjlib/src/pj/os_error_win32.c',
            '<(pjsip_source)/pjlib/src/pj/os_time_win32.c',
            '<(pjsip_source)/pjlib/src/pj/os_timestamp_win32.c',
            '<(pjsip_source)/pjlib/src/pj/guid_win32.c',
            '<(pjsip_source)/pjlib/src/pj/ip_helper_win32.c',
            '<(pjsip_source)/pjlib/src/pj/unicode_win32.c',
            '<(pjsip_source)/pjlib/src/pj/ioqueue_select.c',
            '<(pjsip_source)/pjlib/src/pj/log_writer_stdout.c',
            '<(pjsip_source)/pjlib/src/pj/addr_resolv_sock.c',
          ],
        }],
        ['os_posix == 1 or OS=="mac"', {
          'sources': [
            '<(pjsip_source)/pjlib/src/pj/file_access_unistd.c',
            '<(pjsip_source)/pjlib/src/pj/file_io_ansi.c',
            '<(pjsip_source)/pjlib/src/pj/os_core_unix.c',
            '<(pjsip_source)/pjlib/src/pj/os_error_unix.c',
            '<(pjsip_source)/pjlib/src/pj/os_time_unix.c',
            '<(pjsip_source)/pjlib/src/pj/os_timestamp_posix.c',
            '<(pjsip_source)/pjlib/src/pj/addr_resolv_sock.c',
            '<(pjsip_source)/pjlib/src/pj/ip_helper_generic.c',
            '<(pjsip_source)/pjlib/src/pj/log_writer_stdout.c',
            '<(pjsip_source)/pjlib/src/pj/os_time_common.c',
            '<(pjsip_source)/pjlib/src/pj/os_timestamp_common.c',
            '<(pjsip_source)/pjlib/src/pj/pool_policy_malloc.c',
            '<(pjsip_source)/pjlib/src/pj/sock_bsd.c',
            '<(pjsip_source)/pjlib/src/pj/sock_select.c',
            '<(pjsip_source)/pjlib/src/pj/ioqueue_select.c',
            '<(pjsip_source)/pjlib/src/pj/guid_simple.c',
          ],
        }],
      ],
    },  # target pjlib
    {
      'target_name': 'pjlib-util',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjlib-util/src/pjlib-util/base64.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/dns.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/dns_dump.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/dns_server.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/errno.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/getopt.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/pcap.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/resolver.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/scanner.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/srv_resolver.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/string.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/stun_simple.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/stun_simple_client.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/symbols.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/xml.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/hmac_md5.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/hmac_sha1.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/md5.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/sha1.c',
        '<(pjsip_source)/pjlib-util/src/pjlib-util/crc32.c',
      ],
    },  # target pjlib-util
    {
      'target_name': 'pjsip-core',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjsip/src/pjsip/sip_auth_aka.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_auth_client.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_auth_msg.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_auth_parser.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_auth_server.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_config.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_dialog.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_endpoint.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_errno.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_msg.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_multipart.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_parser.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_resolve.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_tel_uri.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transaction.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transport.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transport_loop.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transport_tcp.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transport_tls.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_transport_udp.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_ua_layer.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_uri.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_util.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_util_proxy.c',
        '<(pjsip_source)/pjsip/src/pjsip/sip_util_statefull.c',
      ],
    },  # target pjsip-core
    {
      'target_name': 'pjsip-simple',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjsip/src/pjsip-simple/errno.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/evsub.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/evsub_msg.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/iscomposing.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/mwi.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/pidf.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/presence.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/presence_body.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/publishc.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/rpid.c',
        '<(pjsip_source)/pjsip/src/pjsip-simple/xpidf.c',
      ],
    },  # target pjsip-simple
    {
      'target_name': 'pjsip-ua',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_100rel.c',
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_inv.c',
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_reg.c',
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_replaces.c',
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_timer.c',
        '<(pjsip_source)/pjsip/src/pjsip-ua/sip_xfer.c',
      ],
    },  # target pjsip-ua
    {
      'target_name': 'pjsua-lib',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_acc.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_aud.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_call.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_core.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_dump.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_im.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_media.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_pres.c',
        '<(pjsip_source)/pjsip/src/pjsua-lib/pjsua_vid.c',
      ],
    },  # target pjsua-lib
    {
      'target_name': 'pjmedia',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjmedia/src/pjmedia/alaw_ulaw.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/alaw_ulaw_table.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/avi_player.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/bidirectional.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/clock_thread.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/codec.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/conference.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/conf_switch.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/converter.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/converter_libswscale.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/delaybuf.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/dummy.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/echo_common.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/echo_internal.h',
        '<(pjsip_source)/pjmedia/src/pjmedia/echo_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/echo_speex.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/echo_suppress.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/endpoint.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/errno.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/event.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/ffmpeg_util.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/ffmpeg_util.h',
        '<(pjsip_source)/pjmedia/src/pjmedia/format.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/g711.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/jbuf.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/master_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/mem_capture.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/mem_player.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/null_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/plc_common.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/resample_libsamplerate.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/resample_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/resample_resample.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/resample_speex.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/rtcp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/rtcp_xr.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/rtp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/sdp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/sdp_cmp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/sdp_neg.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/session.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/silencedet.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/sound_legacy.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/sound_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/splitcomb.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/stereo_port.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/stream.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/stream_common.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/stream_info.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/tonegen.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/transport_adapter_sample.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/transport_ice.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/transport_loop.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/transport_srtp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/transport_udp.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/types.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/wave.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/wav_player.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/wav_playlist.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/wav_writer.c',
        '<(pjsip_source)/pjmedia/src/pjmedia/wsola.c',
      ],
    },  # target pjmedia
    {
      'target_name': 'pjmedia-codec',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/amr_sdp_match.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/audio_codecs.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/g722.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/g7221.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/g7221_sdp_match.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/gsm.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/h263_packetizer.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/h264_packetizer.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/ilbc.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/ipp_codecs.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/l16.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/opencore_amr.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/passthrough.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/speex_codec.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-codec/silk.c',
      ],
    },  # target pjmedia-codec
    {
      'target_name': 'pjmedia-audiodev',
      'type': 'static_library',
      'sources': [
        '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/audiodev.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/audiotest.c',
        '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/errno.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/wmme_dev.c',
          ],
          'defines': [
            'PJMEDIA_AUDIO_DEV_HAS_WMME=1'
          ]
        }, { # OS!="win"
          'defines': [
            'PJMEDIA_AUDIO_DEV_HAS_WMME=0',
          ],
        }],
        ['OS=="linux"', {
          'sources': [
            '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/alsa_dev.c',
          ],
          'defines': [
            'PJMEDIA_AUDIO_DEV_HAS_ALSA=1'
          ]
        }],
        ['OS=="mac"', {
          'sources': [
            '<(pjsip_source)/pjmedia/src/pjmedia-audiodev/coreaudio_dev.c',
          ],
          'defines': [
            'PJMEDIA_AUDIO_DEV_HAS_COREAUDIO=1'
          ]
        }],
        ['OS=="android"', {
          'defines': [
            'PJ_ANDROID=1'
          ]
        }],
      ],
    },  # target pjmedia-audiodev
  ],
}
