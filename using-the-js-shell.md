---
layout: page
title: Using the Javascript Shell
description: Use the Javascript Shell (v8) to test the phone library
---

# Prerequisites

After building Sippet for your platform, you will find an executable named
'sippet_phone_v8_shell' in the output directory.

When you run it without parameters, you might get a shell line the following:

    Sippet shell (V8 version 4.2.29)
    > 


# Using the shell

## Creating a Phone object

The iterative shell offers a Phone object, which you can instantiate as follows

{% highlight js %}
var phone = sippet.Phone();
{% endhighlight %}


## Initialize the Phone object

Before using the Phone object, it has to be initialized. The simplest
initialization is the following:

{% highlight js %}
phone.init();
{% endhighlight %}

But you can pass other configurations:

{% highlight js %}
var settings = {
  // disable DTLS transport while streaming
  disable_encryption: true,
  ice_servers: [
    // a simple STUN server
    { uri: "stun:stun.l.google.com:19302" },

    // a TURN server for port allocations
    {
      uri: "turn:hello.com?transport=tcp",
      username: "test",
      password: "pass"
    }
  ]
};
phone.init(settings);
{% endhighlight %}


## Phone properties

{% highlight js %}
// Phone state:
//  0 = offline
//  1 = connecting
//  2 = online
console.log(String(phone.state));
{% endhighlight %}


## Events definition

Full list of Phone events.


### Login completed

{% highlight js %}
phone.on('loginCompleted', function(state_code, status_text){
  /* Your code here */
});
{% endhighlight %}


### Incoming call

{% highlight js %}
phone.on('incomingCall', function(call){
  /* Your code here */
});
{% endhighlight %}


### Call Ringing

{% highlight js %}
phone.on('callRinging', function(call){
  /* Your code here */
});
{% endhighlight %}


### Call Established

{% highlight js %}
phone.on('callEstablished', function(call){
  /* Your code here */
});
{% endhighlight %}


### Call Hung Up

{% highlight js %}
phone.on('callHungUp', function(call){
  /* Your code here */
});
{% endhighlight %}


### Call Error

{% highlight js %}
phone.on('callError', function(status_code, status_text, call){
  /* Your code here */
});
{% endhighlight %}


### Network Error

{% highlight js %}
phone.on('networkError', function(error_code){
  /* Your code here */
});
{% endhighlight %}


## Making Login

Now, you may want to login before receiving and making calls.

{% highlight js %}
var account = {
  username: "test",
  password: "1234",
  host: "sip:host.com;transport=tcp" // Registrar SIP-URI
};
phone.login(account);
{% endhighlight %}


## Making calls

In order to call someone at the same Registrar server you have made login,
execute:

{% highlight js %}
var call = phone.makeCall("1234");
{% endhighlight %}

In the example above, a call will be initiated to
`sip:1234@host.com;transport=tcp`.


### Some call properties

The following properties are available:

{% highlight js %}
// Call Type:
//   0 = incoming,
//   1 = outgoing
console.log(String(call.type));

// Call State:
//   0 = calling,
//   1 = ringing,
//   2 = established,
//   3 = hungup,
//   4 = error
console.log(String(call.state));

// Call full URI
//  "sip:dest@host.com"
console.log(call.uri);

// Call destination name
//  "dest"
console.log(call.name);

// Time when the call was created
console.log(call.creationTime);

// Time when the call was established
console.log(call.startTime);

// Time when the call was finished
console.log(call.endTime);

// Call duration in seconds
console.log(String(call.duration));
{% endhighlight %}


### Answering an incoming call

{% highlight js %}
call.answer();
{% endhighlight %}


### Send a DTMF digit

{% highlight js %}
call.sendDtmf("1234");
{% endhighlight %}


### Hangup a call

{% highlight js %}
call.hangup();
{% endhighlight %}


## Hangup all calls

{% highlight js %}
phone.hangupAll();
{% endhighlight %}

## Logout

{% highlight js %}
phone.logout();
{% endhighlight %}

