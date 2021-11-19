# OS_3

	>>>gcc main.c -o main 
	>>>./main 
-Komutlari ile derlenir ve calistirilir. 

### Kodun Yaptigi Is ###
- oncelikle 500x2048 boyutlarinda bir char array (memory) dinamik olarak allocate edilir
- tasks.txt dosyasi satir satir okunur
	+ her satir whitespace delimiter ile token'lara ayrilir
	+ ilk token task name, kalan tokenlar ise access index olarak parse edilir ve bir structre'da saklanir
- processler (tasklar) memory'e yuklenir
	+ tasks.txt dosyasinda tanimlanmis tum task dosyalari icin:
		* eger frame tablosunda bu task icin (a.txt vb.) yer varsa tanimlanan task dosyasi (a.txt vb.) acilir
			** frame size kadar (2048) byte okunur
			** frame table'dan random olarak bos bir frame secilir
			** random olarak secilen bu frame'e (memory) okunan byte'lar yazilir
			** dosya tamamen okunana kadar bu islem devam eder
		* frame tablosunda, bu task icin yeterli alan yoksa bir sonraki task'a gecilir
- odevin ilk kismi olan her task icin frame tablosu gosterilir
	+ tasks.txt'de tanimlanmis her task icin su bilgiler ekrana yazilir:
		* task adi ve task dosyasinin boyutu
		* task, sisteme kabul edildiyse bu task icin random atanmis tum frame numaralari
		* taskin sisteme kabul edilip edilmedigi bilgisi
- odevin ikinci kismi olan scheduling simulasyonuna baslanir
	+ her task icin, tasks.txt dosyasinda tanimlanmis access index'lere 10'ar tane erisim yapilir
	+ erisilen byte'lar frame numarasi ve istenen byte'in o frame'deki byte offset bilgisiyle ekrana yazilir.
	+ ilgili task dosyasinda mevcut olmayan bir byte'a erisilmek istendigi zaman gerekli uyari mesaji ekrana basilir.
	+ tasks.txt dosyasinda tanimlanmis tum access index'lere erisilene kadar bu islem 10'ar 10'ar tum sirayla tum tasklar icin devam eder.
	+ NOT: her seferinde 10 access yapilmasi gerektigi icin a.txt taskina 23, b.txt tasina 6 tane access index yazilirsa program su sekilde calisacaktir:
		a.txt 10 valid access
		b.txt 6 valid access, tum accessler yapildigi icin kullaniciya 4 tane 'erisimler tamamlandi' uyarisi
		a.txt 10 valid access
		b.txt 10 tane 'erisimler tamamlandi' uyarisi
		a.txt 3 valid access, tum accessler yapildigi icin kullaniciya 7 tane 'erisimler tamamlandi' uyarisi
		Uyarilar, bir hata veya bir koddaki bir yanlis oldugu icin degil, scheduling'deki IDLE job gibi dusunulerek kullaniciya bilgi amacli verilmistir.
- allocate edilen memory serbest birakilir		

### Varsayimlar ###
- Input (task) dosyasi adi: tasks.txt
- Input (task) dosyasinda satir uzunlugu: 1000 karakter
	+ her satirin maksimum 1000 karakterden olusacagi varsayilmistir
		ornegin: a.txt 1 2 3 .....
	+ dosyadaki satir sayisi limitinden (yani task sayisi) asagida bahsedilmistir
- Input (task) dosyasindaki task isimleri: 256 karakter
	+ her task adinin maksimum 256 karakterdan olusacagi varsayilmistir
		ornegin a.txt, bbbbb.txt, ddddddddddddddddd.txt
- Input (task) dosyasinda tanimli task sayisi: 100


	+ maksimum 100 tane task tanimlinacagi varsayilmistir
