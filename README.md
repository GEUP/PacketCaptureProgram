# PacketCaptureProgram
리눅스 기반 패킷캡쳐 프로그램

PacketCaptureProgram using C Standard Library on Linux



### 1. 프로그램 설계도
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126118667-f7b7cdb3-c705-4ea1-a669-ec080e3b850d.png)





### 2. 구현 순서도
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126118838-7d26c1b9-1c4f-4326-ba3b-cb9d1df6d8f7.png)



 
### 3. 원시 데이터 가공
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126120100-2a6cb77d-402c-4c6e-a492-572841442d8f.png)

위 그림과 같은 raw 데이터 중 각 패킷 별로 핵심이 되는 정보를 저장함


![image](https://user-images.githubusercontent.com/48792627/126120329-d4257af8-604f-4cb7-a14e-641c76f9beb9.png)




### 4. 자료구조 생성
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126120415-103cfc73-a440-44a2-9a93-9026d8ada9fc.png)

추출한 핵심 정보를 생성한 자료구조에 저장


![image](https://user-images.githubusercontent.com/48792627/126120572-d8e31b1c-56f8-453e-bf5c-f48536087551.png)





### 4. 실행 방법
------------------------------

1. sudo ./packetcapture

2. 캡쳐시작 (y/n) :  y

3. 웹페이지 방문 (http://www.kpu.ac.kr/index.do?sso=ok) 

4. 별도의 다른 터미널에서 다음 명령어 수행
    - ping google.com
    - nslookup kpu.ac.kr

5. ctrl + c 입력 - 패킷 수집 종료

6. 패킷 검색

 
### 5. 실행 화면
------------------------------

# 시작 화면

![image](https://user-images.githubusercontent.com/48792627/126118937-375a773a-32e0-4681-a0aa-4d3ef1c35045.png)

Y : 패킷 캡쳐 시작
N : 프로그램 종료



# 캡쳐 시작 화면

![image](https://user-images.githubusercontent.com/48792627/126119027-9f247960-416e-4adf-936e-1d3b25f90f7f.png)

ctrl + c 입력 전까지 모든 패킷을 수집



# 패킷 캡쳐 내역 저장

- origin.txt

![image](https://user-images.githubusercontent.com/48792627/126119216-09b5b83d-f0af-4f38-9820-3f9d855ab7fe.png)



- summary.txt

![image](https://user-images.githubusercontent.com/48792627/126119234-a9ee2264-4ed6-4704-a948-c6e67ba3623b.png)



캡쳐한 패킷을 터미널에서 출력함과 동시에 파일에 저장한다.
orign.txt : 인덱스번호, 패킷의 크기, 원시코드
summary.txt : 패킷 요약 저장


# 패킷 캡쳐 종료

![image](https://user-images.githubusercontent.com/48792627/126119336-77fe598b-de33-494b-aba9-e60e0ca197ce.png)


시그널핸들러를 이용해 ctrl+c 입력시 검색으로 넘어가도록 구현

![image](https://user-images.githubusercontent.com/48792627/126119393-a25df89a-e3cc-47b6-b6fe-7374d1ab89d2.png)




# 일반 검색

![image](https://user-images.githubusercontent.com/48792627/126119494-c1774b46-44df-4653-8c6e-d609a1e4d90e.png)


* 인덱스 검색

![image](https://user-images.githubusercontent.com/48792627/126119565-fb85e18a-3278-42cf-ba1b-6562f07045ee.png)



* 프로토콜, IP번호, PORT 번호 검색

![image](https://user-images.githubusercontent.com/48792627/126119588-1def4b02-4008-4aaf-96a9-f21bfed55f3a.png)

-1을 입력하지 않은 정보를 바탕으로 검색





# 특수 검색

![image](https://user-images.githubusercontent.com/48792627/126119650-790ab5d9-8075-4355-9fad-490cadfe12e8.png)



* HTTP 특수 검색


- 3단계 핸드쉐이크

![image](https://user-images.githubusercontent.com/48792627/126119795-6ea58607-fc69-4f9c-8555-f1cc8c2414b0.png)


- 요청메시지, 응답메시지

![image](https://user-images.githubusercontent.com/48792627/126119844-82054bf4-6ae1-46f4-856e-0835e0964a1d.png)


- 4단계 핸드쉐이크

![image](https://user-images.githubusercontent.com/48792627/126119897-38c79c5f-ccf1-40b5-88b1-e915abb88f03.png)




* ICMP 특수 검색

![image](https://user-images.githubusercontent.com/48792627/126119946-c2a3b5a0-6161-4a6e-bc04-fe53c46bea60.png)


* DNS 특수 검색

![image](https://user-images.githubusercontent.com/48792627/126119987-32fac74e-1c59-4d2f-aa89-de30cec07889.png)



