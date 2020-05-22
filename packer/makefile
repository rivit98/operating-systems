CW_NUM=10

all:
	cp -r ../GierlachAlbert/cw$(CW_NUM) .
	rm -rf GierlachAlbert-cw$(CW_NUM).tar.gz
	mkdir GierlachAlbert
	cp -r cw$(CW_NUM) ./GierlachAlbert
	find ./GierlachAlbert -type f -iname "*.pdf" -delete
	tar cvzf GierlachAlbert-cw$(CW_NUM).tar.gz GierlachAlbert
	rm -rf GierlachAlbert cw$(CW_NUM)