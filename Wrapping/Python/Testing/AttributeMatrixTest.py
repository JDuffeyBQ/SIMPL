from dream3d import simpl
import simpl_helpers as sh

def AttributeMatrixTest():
    # Create an AttributeMatrix
    amType = simpl.AttributeMatrix.Type.Cell
    tupleDims = simpl.VectorSizeT([5,4,3])
    amName = 'CellAttributeMatrix'
    am = sh.CreateAttributeMatrix(tupleDims, amName, amType)

    print('Name: %s' % am.Name)

    arrayExist = am.doesAttributeArrayExist('Test Data Array')
    assert arrayExist is False
    print('arrayExist: %s' % arrayExist)

if __name__ == '__main__':
    AttributeMatrixTest()
    print('AttributeMatrixTest Test Complete')
