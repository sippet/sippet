---
layout: page
title: Using fluent SIP Messages
description: Discover how the fluent Sippet messages work
---

Sippet messages are a set of classes that gives the high-level application
programmer a fluent usage of the request and response data. Messages can be of
type `Request` or `Response`, and are composed of `Header` objects and a
content body.

Instead of parsing each SIP header by yourself, or creating each one by
composing strings following RFCs specifications, this is delegated to an
hierarchy of classes, all derived from `Header`.


# Creating Messages

Here is an example of a full INVITE request creation:

{% gist guibv/791a364569cd77b3ef47 %}

This is a copy of the example message given originally in RFC 3261:

    INVITE sip:bob@biloxi.com SIP/2.0
    Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bK776asdhds
    Max-Forwards: 70
    To: Bob <sip:bob@biloxi.com>
    From: Alice <sip:alice@atlanta.com>;tag=1928301774
    Call-ID: a84b4c76e66710@pc33.atlanta.com
    CSeq: 314159 INVITE
    Contact: <sip:alice@pc33.atlanta.com>
    Content-Type: application/sdp
    Content-Length: 0

The response generation follows about the same sequence, with the difference
you have to use a `Response` instead of a `Request` type.

The Sippet library offers message builders to ease the work for you. Check
classes `sippet::ua::UserAgent` and `sippet::Dialog`.


# SIP and Tel URIs canonicalization

Sippet comes with `SipURI` and `TelURI` classes for URI canonicalization. Both
accept conversions from GURL, but it's required to check the scheme first:

{% gist guibv/02100b0b57befe8b79ca %}

It's possible to "convert" a `TelURI` into a `SipURI`. For example, the URI
**tel:+1555000000** can be converted into
**sip:+1555000000@biloxi.com;user=phone** using the following code:

{% gist guibv/7b2adf298c20b05e2475 %}

If you have to convert a `SipURI` into a `GURL`, the following code can be
used:

{% gist guibv/fd174e04a75daf4186ae %}

