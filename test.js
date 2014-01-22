var d=require('./build/Release/dht22');

console.log(d);

var data=d.read(4);

console.log(data);

