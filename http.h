//
// Created by helidium on 3/28/17.
//
// Parts of the code are based on software developed as µWebSockets,
// Copyright (c) 2016 Alex Hultman and contributors.
//

#ifndef MNS_HTTP_H
#define MNS_HTTP_H

#define SERVER_CLOSE_EVENT 1
#define SERVER_CONNECTION_EVENT 2
#define SERVER_ERROR_EVENT 3
#define SERVER_LISTENING_EVENT 4

#include <node.h>
#include <v8.h>
#include "./src/Server.h"

using namespace v8;

Persistent<Object> requestObjectTemplate, responseObjectTemplate;
Persistent<Object> httpPersistent;
Persistent<Function> httpRequestCallback;

struct HttpRequest {
	static void On(const FunctionCallbackInfo<Value> &args) {
		if (args.Length() == 2 && args[0]->IsString()) {
			String::Utf8Value EventName(args[0]);
			if (strncmp(*EventName, "data", 4) == 0) {
				args.Holder()->SetInternalField(1, args[1]);
			} else if (strncmp(*EventName, "end", 3) == 0) {
				args.Holder()->SetInternalField(2, args[1]);
			}
		}
	}

	static void RemoveListener(const FunctionCallbackInfo<Value> &args) {
	}

	static void Unpipe(const FunctionCallbackInfo<Value> &args) {

	}

	static void Resume(const FunctionCallbackInfo<Value> &args) {

	}

	static void GetUrl(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		MNS::SocketData *data = static_cast<MNS::SocketData *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		if(data) {
			args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) data->request->url));
		}
	}

	static void GetMethod(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		MNS::SocketData *data = static_cast<MNS::SocketData *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		if(data) {
			switch (data->request->method) {
				case MNS::HTTP_METHOD::GET:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "GET", String::kNormalString, 3));
					break;
				case MNS::HTTP_METHOD::POST:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "POST", String::kNormalString, 4));
					break;
				case MNS::HTTP_METHOD::HEAD:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "HEAD", String::kNormalString, 4));
					break;
				case MNS::HTTP_METHOD::OPTIONS:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "OPTIONS", String::kNormalString, 7));
					break;
				case MNS::HTTP_METHOD::CONNECT:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "CONNECT", String::kNormalString, 7));
					break;
				case MNS::HTTP_METHOD::DELETE:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "DELETE", String::kNormalString, 6));
					break;
				case MNS::HTTP_METHOD::PATCH:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "PATCH", String::kNormalString, 5));
					break;
				case MNS::HTTP_METHOD::PUT:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "PUT", String::kNormalString, 3));
					break;
				case MNS::HTTP_METHOD::TRACE:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "TRACE", String::kNormalString, 5));
					break;
				case MNS::HTTP_METHOD::UNKNOWN_METHOD:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "UNKNOWN", String::kNormalString, 8));
					break;
			}
		}
	}

	static void GetHttpVersion(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		MNS::SocketData *data = static_cast<MNS::SocketData *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		if(data) {
			switch (data->request->httpVersion) {
				case MNS::HTTP_VERSION::HTTP_1_0:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "1.0", String::kNormalString, 3));
					break;
				case MNS::HTTP_VERSION::HTTP_1_1:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "1.1", String::kNormalString, 3));
					break;
				case MNS::HTTP_VERSION::UNKNOWN_VERSION:
					args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) "UNKNOWN", String::kNormalString, 8));
					break;
			}
		}
	}

	static void GetHeader(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		String::Utf8Value name(property);

		MNS::SocketData *data = static_cast<MNS::SocketData *>(args.Holder()->GetPrototype()->ToObject()->GetAlignedPointerFromInternalField(0));

		if(data) {
			std::map<std::string, std::string>::const_iterator it = data->request->headers.find(std::string(*name, name.length()));

			if (it != data->request->headers.end()) {
				args.GetReturnValue().Set(String::NewFromOneByte(args.GetIsolate(), (uint8_t *) it->second.c_str(), String::kNormalString, it->second.length()));
			} else {
				args.GetReturnValue().Set(Undefined(args.GetIsolate()));
			}
		}
	}

	static void GetSocket(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		// TODO
		args.GetReturnValue().Set(Object::New(args.GetIsolate()));
	}

	static Local<Object> getObjectTemplate(Isolate *isolate) {
		Local<ObjectTemplate> proto = ObjectTemplate::New(isolate);
		proto->SetInternalFieldCount(3);
		Local<Object> proto_i = proto->NewInstance();

		Local<ObjectTemplate> headersTemplate = ObjectTemplate::New(isolate);
		headersTemplate->SetNamedPropertyHandler(HttpRequest::GetHeader);

		Local<ObjectTemplate> obj_t = ObjectTemplate::New(isolate);
		obj_t->SetInternalFieldCount(3); // 0->SocketData;1->data callback;2->end callback

		obj_t->Set(isolate, "unpipe", FunctionTemplate::New(isolate, HttpRequest::Unpipe));
		obj_t->Set(isolate, "resume", FunctionTemplate::New(isolate, HttpRequest::Resume));
		obj_t->Set(isolate, "on", FunctionTemplate::New(isolate, HttpRequest::On));
		obj_t->Set(isolate, "removelistener", FunctionTemplate::New(isolate, HttpRequest::RemoveListener));
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "httpVersion"), HttpRequest::GetHttpVersion);
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "method"), HttpRequest::GetMethod);
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "url"), HttpRequest::GetUrl);
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "socket"), HttpRequest::GetSocket);

		Local<Object> obj_i = obj_t->NewInstance();
		obj_i->SetPrototype(proto_i);

		Local<Object> ht_i = headersTemplate->NewInstance();
		ht_i->SetPrototype(proto_i);

		obj_i->Set(String::NewFromUtf8(isolate, "headers"), ht_i);

		return obj_i;
	}
};

struct HttpResponse {
	static void Write(const FunctionCallbackInfo<Value> &args) {
		MNS::SocketData *data = static_cast<MNS::SocketData *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		// TODO: Make sure it is heap allocated; GC not released the value
		if(data) {
			if(!(args[0]->IsNull() || args[0]->IsUndefined())) {
				String::Utf8Value str(args[0]);
				data->response->write(*str, str.length());
			}
		}
	}

	static void End(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		MNS::SocketData *data = (MNS::SocketData *) args.Holder()->GetAlignedPointerFromInternalField(0);

		if(data) {
			// Release the handle
			static_cast<Persistent<Object> *>(data->nodeRequestPlaceholder)->Reset();
			delete (static_cast<Persistent<Object> *>(data->nodeRequestPlaceholder));
			//static_cast<Persistent<Object> *>(data->nodeRequestPlaceholder)->~Persistent<Object>();

			static_cast<Persistent<Object> *>(data->nodeResponsePlaceholder)->Reset();
			delete (static_cast<Persistent<Object> *>(data->nodeResponsePlaceholder));
			//static_cast<Persistent<Object> *>(data->nodeResponsePlaceholder)->~Persistent<Object>();

			if(args[0]->IsNull() || args[0]->IsUndefined()) {
				data->response->end(nullptr, 0);
			} else {
				// TODO: Make sure it is heap allocated; GC not released the value
				String::Utf8Value str(args[0]);
				data->response->end(*str, str.length());
			}

			data->nodeRequestPlaceholder = nullptr;
			data->nodeResponsePlaceholder = nullptr;

			Local<Value> finishCallback = args.Holder()->GetInternalField(1);
			if (!finishCallback->IsUndefined()) {
				Local<Function>::Cast(finishCallback)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
			}

			Local<Value> endCallback = args.Holder()->GetInternalField(2);
			if (!endCallback->IsUndefined()) {
				Local<Function>::Cast(endCallback)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
			}

			args.Holder()->SetAlignedPointerInInternalField(0, nullptr);
		}
	}

	static void AddTrailers(const FunctionCallbackInfo<Value> &args) {
	}

	static void IsFinished(Local<String> property, const PropertyCallbackInfo<Value> &args) {
		MNS::SocketData *data = (MNS::SocketData *) args.Holder()->GetAlignedPointerFromInternalField(0);

		if(data) {
			args.GetReturnValue().Set(Boolean::New(args.GetIsolate(), data->response->finished));
		}
	}

	static void SetHeader(const FunctionCallbackInfo<Value> &args) {
		if (args.Length() == 2 && args[0]->IsString() && args[1]->IsString()) {
			MNS::SocketData *data = (MNS::SocketData *) args.Holder()->GetAlignedPointerFromInternalField(0);

			if(data) {
				String::Utf8Value name(args[0]->ToString());
				String::Utf8Value value(args[1]->ToString());

				data->response->setHeader(std::string(*name, name.length()), std::string(*value, value.length()));
			}
		}
	}

	static void GetStatusCode(Local<String> property, const PropertyCallbackInfo<Value>& info) {
		MNS::SocketData *data = (MNS::SocketData *) info.Holder()->GetAlignedPointerFromInternalField(0);

		info.GetReturnValue().Set(data->response->statusCode);
	}

	static void SetStatusCode(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
		MNS::SocketData *data = (MNS::SocketData *) info.Holder()->GetAlignedPointerFromInternalField(0);

		data->response->statusCode = value->Int32Value();
	}

	static void WriteHead(const FunctionCallbackInfo<Value> &args) {
		printf("Writing Head\n");
	}

	static void SetTimeout(const FunctionCallbackInfo<Value> &args) {
	}

	static void RemoveListener(const FunctionCallbackInfo<Value> &args) {}

	static void On(const FunctionCallbackInfo<Value> &args) {
		if (args.Length() == 2 && args[0]->IsString()) {
			String::Utf8Value EventName(args[0]);
			if (strncmp(*EventName, "finish", 6) == 0) {
				args.Holder()->SetInternalField(1, args[1]);
			} else if (strncmp(*EventName, "end", 3) == 0) {
				args.Holder()->SetInternalField(2, args[1]);
			}
		}
	}

	static Local<Object> getObjectTemplate(Isolate *isolate) {
		Local<ObjectTemplate> obj_t = ObjectTemplate::New(isolate);
		obj_t->SetInternalFieldCount(3);

		obj_t->Set(isolate, "write", FunctionTemplate::New(isolate, HttpResponse::Write));
		obj_t->Set(isolate, "end", FunctionTemplate::New(isolate, HttpResponse::End));
		obj_t->Set(isolate, "addTrailers", FunctionTemplate::New(isolate, HttpResponse::AddTrailers));
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "finished"), HttpResponse::IsFinished);
		obj_t->SetAccessor(String::NewFromUtf8(isolate, "statusCode"), HttpResponse::GetStatusCode, HttpResponse::SetStatusCode);
		obj_t->Set(isolate, "setHeader", FunctionTemplate::New(isolate, HttpResponse::SetHeader));
		obj_t->Set(isolate, "setTimeout", FunctionTemplate::New(isolate, HttpResponse::SetTimeout));
		obj_t->Set(isolate, "writeHead", FunctionTemplate::New(isolate, HttpResponse::WriteHead));
		obj_t->Set(isolate, "on", FunctionTemplate::New(isolate, HttpResponse::On));
		obj_t->Set(isolate, "removeListener", FunctionTemplate::New(isolate, HttpResponse::RemoveListener));

		return obj_t->NewInstance();
	}
};

struct Http {
	static Local<Object> getObjectTemplate(Isolate *isolate) {
		Local<ObjectTemplate> obj_t = ObjectTemplate::New(isolate);
		obj_t->SetInternalFieldCount(6);

		obj_t->Set(isolate, "on", FunctionTemplate::New(isolate, Http::On));
		obj_t->Set(isolate, "close", FunctionTemplate::New(isolate, Http::stopServer));
		obj_t->Set(isolate, "listen", FunctionTemplate::New(isolate, Http::listen));
		obj_t->Set(isolate, "get", FunctionTemplate::New(isolate, Http::get));
		obj_t->Set(isolate, "request", FunctionTemplate::New(isolate, Http::request));

		return obj_t->NewInstance();
	}

	static void createServer(const FunctionCallbackInfo<Value> &args) {
		MNS::Server *server = new MNS::Server();

		Isolate *isolate = args.GetIsolate();

		Local<Object> obj = Http::getObjectTemplate(isolate);
		obj->SetAlignedPointerInInternalField(0, server);

		httpPersistent.Reset(isolate, obj);

		server->onHttpConnection([isolate](MNS::SocketData *data) {
			HandleScope hs(isolate);

			Local<Value> connectionCallback = httpPersistent.Get(isolate)->GetInternalField(SERVER_CONNECTION_EVENT);

			if(!connectionCallback->IsUndefined()) {
				// TODO: Return the socket object
				Local<Function>::Cast(connectionCallback)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
			}
		});

		if (args.Length() == 1 && args[0]->IsFunction()) {
			httpRequestCallback.Reset(isolate, Local<Function>::Cast(args[0]));

			server->onHttpRequest([isolate](MNS::SocketData *data) {
				HandleScope hs(isolate);

				Local<Object> req = Local<Object>::New(isolate, requestObjectTemplate)->Clone();
				req->SetAlignedPointerInInternalField(0, data);
				Local<Object> reqProto = req->GetPrototype()->ToObject();
				reqProto->SetAlignedPointerInInternalField(0, data);

				Local<Object> res = Local<Object>::New(isolate, responseObjectTemplate)->Clone();
				res->SetAlignedPointerInInternalField(0, data);
				Local<Value> argv[] = {
					req,
					res
				};

				data->nodeRequestPlaceholder = new Persistent<Object>(isolate, req);
				data->nodeResponsePlaceholder = new Persistent<Object>(isolate, res);

				Local<Function>::New(isolate, httpRequestCallback)->Call(isolate->GetCurrentContext()->Global(), 2, argv);

				Local<Value> dataCallback = req->GetInternalField(1);
				if (!dataCallback->IsUndefined()) {
					Local<Value> argv[] = {String::NewFromUtf8(isolate, data->request->getBodyBuffer(), v8::String::kNormalString, data->request->getBodyBufferLen())};
					Local<Function>::Cast(dataCallback)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
				}

				Local<Value> endCallback = req->GetInternalField(2);
				if (!endCallback->IsUndefined()) {
					Local<Function>::Cast(endCallback)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
				}

				isolate->RunMicrotasks();
			});

			server->onHttpCancel([isolate](MNS::SocketData *data) {
				HandleScope hs(isolate);

				if(data->nodeRequestPlaceholder) {
					Local<Object> reqObject = Local<Object>::New(isolate, *(Persistent<Object> *)data->nodeRequestPlaceholder);
					reqObject->SetAlignedPointerInInternalField(0, nullptr);
					reqObject->GetPrototype()->ToObject()->SetAlignedPointerInInternalField(0, nullptr);

					// Release the handle
					static_cast<Persistent<Object> *>(data->nodeRequestPlaceholder)->Reset();
					delete(static_cast<Persistent<Object> *>(data->nodeRequestPlaceholder));
				}

				if(data->nodeResponsePlaceholder) {
					Local<Object> resObject = Local<Object>::New(isolate, *(Persistent<Object> *)data->nodeResponsePlaceholder);
					resObject->SetAlignedPointerInInternalField(0, nullptr);

					// Release the handle
					static_cast<Persistent<Object> *>(data->nodeResponsePlaceholder)->Reset();
					delete(static_cast<Persistent<Object> *>(data->nodeResponsePlaceholder));
				}
			});
		}

		requestObjectTemplate.Reset(isolate, HttpRequest::getObjectTemplate(isolate));
		responseObjectTemplate.Reset(isolate, HttpResponse::getObjectTemplate(isolate));

		args.GetReturnValue().Set(obj);
	}

	static void stopServer(const FunctionCallbackInfo<Value> &args) {
		MNS::Server *server = static_cast<MNS::Server *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		if(server) {
			server->stop();
		}
	}

	static void On(const FunctionCallbackInfo<Value> &args) {
		if (args.Length() == 2 && args[0]->IsString()) {
			String::Utf8Value EventName(args[0]);
			if (strncmp(*EventName, "close", 5) == 0) {
				args.Holder()->SetInternalField(SERVER_CLOSE_EVENT, args[1]);
			} else if (strncmp(*EventName, "connection", 10) == 0) {
				args.Holder()->SetInternalField(SERVER_CONNECTION_EVENT, args[1]);
			} else if (strncmp(*EventName, "error", 5) == 0) {
				args.Holder()->SetInternalField(SERVER_ERROR_EVENT, args[1]);
			} else if (strncmp(*EventName, "listening", 10) == 0) {
				args.Holder()->SetInternalField(SERVER_LISTENING_EVENT, args[1]);
			}
		}
	}

	static void listen(const FunctionCallbackInfo<Value> &args) {
		MNS::Server *server = static_cast<MNS::Server *>(args.Holder()->GetAlignedPointerFromInternalField(0));

		// Commence listening
		server->listen(args[0]->IntegerValue());

		// If callback supplied, make the callback
		if (args[args.Length() - 1]->IsFunction()) {
			Local<Function>::Cast(args[args.Length() - 1])->Call(args.GetIsolate()->GetCurrentContext()->Global(), 0, nullptr);
		}

		Isolate::GetCurrent()->RunMicrotasks();
	};

	static void get(const FunctionCallbackInfo<Value> &args) {
		args.GetReturnValue().Set(Null(args.GetIsolate()));
	}

	static void request(const FunctionCallbackInfo<Value> &args) {
		args.GetReturnValue().Set(Null(args.GetIsolate()));
	}
};

#endif //MNS_HTTP_H
