mkdir android

#http://developer.android.com/sdk/index.html
curl -L -O http://dl.google.com/android/android-sdk_r24.3.3-macosx.zip
mv android-sdk_r24.3.3-macosx.zip android/
cd android && unzip android-sdk_r24.3.3-macosx.zip && cd ..

#https://developer.android.com/tools/sdk/ndk/index.html
curl -L -O http://dl.google.com/android/ndk/android-ndk-r10e-darwin-x86_64.bin
mv android-ndk-r10e-darwin-x86_64.bin android/
chmod +x android/android-ndk-r10e-darwin-x86_64.bin
cd android/ && ./android-ndk-r10e-darwin-x86_64.bin


