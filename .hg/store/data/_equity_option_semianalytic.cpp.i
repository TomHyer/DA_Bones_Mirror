        �  %       	��������,O��C�Gܧ�[pA�xOS            x�uR�j�@=K���`�A����C-��IR_JXƫq�DҪ����߳�r@�i��7ｙ�,��[U���F�ۜ?�f����^��cm�j4�XX�	솭V��uO:~���I��bES�h�:��b�xI+H(`� +��_�&;cjX1Sӱ�M��\osP�uQX΀C�C�X�.}�	q\�b�cݠņ�����%��ʥ�(:�v����*��j*�3����{xd�1XL�gM��@uh��J*t�D����=+�,:�uG�o�E�:�f �q��B�A��R<=�I4|&�%�m{o�"x�(.�Q��8|x�ع(Ǡ�5Y��� ��(��=��G��#ɰ1�:����~�N��x���(�ˊ��|>����������k���߯Vb|EqM����-�}�r��_ ��C    �     W          
    ����?�+��qI@����n���              2  �   K		 const ValuationParameters_*,
		 Vector_<pair<String_, double>>* vals)
    �     Y  �          �����j�^-��z��� �m��o�              7  �   M		vals->push_back(make_pair(trade.valueNames_[0], myModel->Price(*myOpt)));
    O     L  �          �����5	��Y!�����_��F��              �  �   @RUN_AT_LOAD(Semianalytic::Register(new PriceEquityOption_, 5))
    �     Q  �          ����9��[���b�������3              7  �   E		vals->emplace_back(trade.valueNames_[0], myModel->Price(*myOpt));
