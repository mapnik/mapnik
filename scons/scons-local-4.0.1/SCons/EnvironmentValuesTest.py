import unittest

from  SCons.EnvironmentValues import EnvironmentValues

class MyTestCase(unittest.TestCase):
    def test_simple_environmentValues(self):
        """Test comparing SubstitutionEnvironments
        """

        env1 = EnvironmentValues(XXX='x')
        env2 = EnvironmentValues(XXX='x',XX="$X", X1="${X}", X2="$($X$)")



if __name__ == '__main__':
    unittest.main()
