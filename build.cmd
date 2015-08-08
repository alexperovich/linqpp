@echo off
pushd %~dp0

IF NOT exist bin (
  mkdir bin
)
pushd bin

cmake ..

popd

msbuild .\bin\LinqPP.sln /m /p:Configuration=Release /v:q %*

popd
