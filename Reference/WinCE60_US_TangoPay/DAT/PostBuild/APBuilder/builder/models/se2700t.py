import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '1024_600'

class SE2700T(BaseModel):
    """
    A builder for the NH2700T

    """

    def __init__(self):
        super(SE2700T, self).__init__('2700', [
                ('2700', SCREEN_SIZE, '*'),
                ('2700', '720_480', 'OP_*')
            ], '2700T')
