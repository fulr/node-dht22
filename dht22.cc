#include <node.h>
#include <v8.h>

#include "dht22.h"

using namespace v8;

Handle<Value> ReadDHT22(const Arguments& args) {
  HandleScope scope;

  Local<Object> obj=Object::New();

  float humidity;
  float temperature;

  int state=read_dht22_dat(7,&humidity,&temperature);

  obj->Set(String::New("state"),Number::New(state));
  obj->Set(String::New("humidity"),Number::New(humidity));
  obj->Set(String::New("temperature"),Number::New(temperature));

  return scope.Close(obj);
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("read"),
      FunctionTemplate::New(ReadDHT22)->GetFunction());
}

NODE_MODULE(dht22, init)

