var fs = require('fs');
//Check Format
if(process.argv.length-2 != 3){
  console.log('Usage: ./dofile.js <aag> <dofile> <testCase>');
  process.exit(1);
}
//setting
const R = 10;//testRandomGate number

var aagFile  = process.argv[2];
var doFile = process.argv[3];
var testCase = process.argv[4];
var miloa = [];
var out = fs.createWriteStream(doFile , {
  encoding: 'utf8'
});

// utils function
function preProcess(){
  var ctx = fs.readFileSync(aagFile,'utf8');
  var data = /^aag\s(\d+)\s(\d+)\s(\d+)\s(\d+)\s(\d+)/g.exec(ctx);
  miloa = [data[1],data[2],data[3],data[4],data[5]];
}
function testStructure(){
  out.write('cirp\n');
  out.write('cirp -n\n');
  out.write('cirp -fl\n');
  out.write('cirw\n');
}
function testAllGate(){
  for(var j = 0; j <= miloa[0]; j++)
    out.write(`cirg ${j}\n`);
  for(var j = 0; j <= miloa[0]; j++)
    out.write(`cirg ${j} -fanout 100\n`);
  for(var j = 0; j <= miloa[0]; j++)
    out.write(`cirg ${j} -fanin 100\n`);
}
function testRandomGate(i){
  var m = miloa[0];
  for(var j = 0; j <= i; j++)
    out.write(`cirg ${Math.floor(Math.random()*m)}\n`);
  // for(var j = 0; j <= i; j++)
  //   out.write(`cirg ${Math.floor(Math.random()*m)} -fanout 100\n`);
  for(var j = 0; j <= i; j++)
    out.write(`cirg ${Math.floor(Math.random()*m)} -fanin 100\n`);
}
function testFec(){
  // PRINT GATE
  for(var j = 0; j <= miloa[0]; j++)
    out.write(`cirg ${j}\n`);
  // PRINT FEC
  out.write(`cirp -fec\n`);
}

function cirbasic(){
  testStructure();
  testRandomGate(100);
}
// different case
function cirsw(){
  out.write('cirsw\n');
  testStructure();
  testRandomGate(10);
}

function ciropt(){
  out.write('ciropt\n');
  testStructure();
  testRandomGate(100);
}

function cirstrash(){
  out.write('cirstrash\n');
  testStructure();
  testRandomGate(100);
}

function cirsimFile(){
  var log = aagFile.replace(/.aag|.\/tests.fraig\//g,'');
  var num = /\d+/.exec(log)[0];
  // console.log(num);
  // var num = aagFile.replace(/.aag/,'');
  out.write(`cirsim -f tests.fraig/pattern.${num} -o ./myoutput/${log}.log\n`);
  out.write(`cirp -fec\n`);
  for(var j = 0; j <= miloa[0]; j++)
    out.write(`cirg ${j}\n`);
  out.write('usage\n');
}

function cirfraig(){
  // if(/sim13/.test(aagFile)) 
  //   return;
  for(var i = 0; i < 3; i++){
    out.write(`cirsim -r\n`);
    out.write(`cirfraig\n`);
  }
  out.write(`cirp -fec\n`);
  // testStructure();
  out.write(`cirsw\n`);
  out.write(`ciropt\n`);
  out.write(`cirstrash\n`);
  // testStructure();
  out.write(`cirp\n`);
  out.write(`cirw -o myoutput/ref.aag\n`)
  out.write('usage\n');
}

preProcess();
out.write(`cirr ${aagFile}\n`);
switch(testCase){
  case "sweep":
    cirsw();
  break;
  case "opt":
    ciropt();
  break;
  case "strash":
    cirstrash();
  break;
  case "sim":
    cirsimFile();
  break;
  case "fraig":
    cirfraig();
  break;
  case "basic":
    cirbasic();
  break;
}
out.write('q -f\n');
