#include "pywrapper.hpp"

#include <richdem/misc/conversion.hpp>
#include <richdem/methods/flow_accumulation.hpp>
#include <richdem/depressions/depression_hierarchy.hpp>
#include <richdem/depressions/fill_spill_merge.hpp>
#include <richdem/common/Array2D.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>

#include <string>
#include <unordered_set>
#include <string>

namespace py = pybind11;

using namespace richdem;

// Déclarez un ensemble pour suivre les types enregistrés
std::unordered_set<std::string> registered_types;

template <typename T>
void CustomTemplatedFunctionsWrapper(py::module &m, const std::string &type_name) {
    // Vérifiez si le type a déjà été enregistré
    if (registered_types.find(type_name) == registered_types.end()) {
        registered_types.insert(type_name);

        // Enregistrez le type ici
        TemplatedFunctionsWrapper<T>(m, type_name);
    }
}

template <typename T>
void CustomTemplatedArrayWrapper(py::module &m, const std::string &type_name) {
    // Vérifiez si le type a déjà été enregistré
    if (registered_types.find(type_name) == registered_types.end()) {
        registered_types.insert(type_name);

        TemplatedArrayWrapper<T>(m, type_name);
    }
}

PYBIND11_MODULE(_richdem, m) {
  m.doc() = "Internal library used by pyRichDEM for calculations";

  m.attr("NO_FLOW") = &richdem::NO_FLOW;

  //py::bind_vector<std::vector<double>>(m, "VecDouble");
  py::bind_map<std::map<std::string, std::string>>(m, "MapStringString");

  CustomTemplatedFunctionsWrapper<float   >(m, "float"   );
  CustomTemplatedFunctionsWrapper<double  >(m, "double"  );
  CustomTemplatedFunctionsWrapper<int8_t  >(m, "int8_t"  );
  CustomTemplatedFunctionsWrapper<int16_t >(m, "int16_t" );
  CustomTemplatedFunctionsWrapper<int32_t >(m, "int32_t" );
  CustomTemplatedFunctionsWrapper<int64_t >(m, "int64_t" );
  CustomTemplatedFunctionsWrapper<uint8_t >(m, "uint8_t" );
  CustomTemplatedFunctionsWrapper<uint16_t>(m, "uint16_t");
  CustomTemplatedFunctionsWrapper<uint32_t>(m, "uint32_t");
  CustomTemplatedFunctionsWrapper<uint64_t>(m, "uint64_t");

  CustomTemplatedArrayWrapper<float   >(m, "float"   );
  CustomTemplatedArrayWrapper<double  >(m, "double"  );
  CustomTemplatedArrayWrapper<int8_t  >(m, "int8_t"  );
  CustomTemplatedArrayWrapper<int16_t >(m, "int16_t" );
  CustomTemplatedArrayWrapper<int32_t >(m, "int32_t" );
  CustomTemplatedArrayWrapper<int64_t >(m, "int64_t" );
  CustomTemplatedArrayWrapper<uint8_t >(m, "uint8_t" );
  CustomTemplatedArrayWrapper<uint16_t>(m, "uint16_t");
  CustomTemplatedArrayWrapper<uint32_t>(m, "uint32_t");
  CustomTemplatedArrayWrapper<uint64_t>(m, "uint64_t");

  m.def("rdHash",        &rdHash,        "Git hash of previous commit");
  m.def("rdCompileTime", &rdCompileTime, "Commit time of previous commit");

  m.def("FlowAccumulation", &FlowAccumulation<double>, "TODO");
  m.def("flow_accumulation_from_d8", &flow_accumulation_from_d8<double>, "TODO");
  m.def("convert_arc_flowdirs_to_richdem_d8", &convert_arc_flowdirs_to_richdem_d8, "Convert ArcGIS Flowdirs to Richdem D8 flowdirs");

  py::class_<Array3D<float>>(m, "Array3D_float", py::buffer_protocol(), py::dynamic_attr())
      .def(py::init<>())
      .def(py::init<typename Array3D<float>::xy_t, typename Array3D<float>::xy_t,float>())

      // .def(py::init<const Array2D<float   >&, T>())
      // .def(py::init<const Array2D<double  >&, T>())
      // .def(py::init<const Array2D<int8_t  >&, T>())
      // .def(py::init<const Array2D<int16_t >&, T>())
      // .def(py::init<const Array2D<int32_t >&, T>())
      // .def(py::init<const Array2D<int64_t >&, T>())
      // .def(py::init<const Array2D<uint8_t >&, T>())
      // .def(py::init<const Array2D<uint16_t>&, T>())
      // .def(py::init<const Array2D<uint32_t>&, T>())
      // .def(py::init<const Array2D<uint64_t>&, T>())

      //NOTE: This does not do reference counting. For that we would want
      //py::object and a wrapped derived class of Array2D
      .def(py::init([](py::handle src){
        // if(!py::array_t<T>::check_(src)) //TODO: What's this about?
          // return false;

        auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(src);
        if (!buf)
          throw std::runtime_error("Unable to convert array to RichDEM object!");

        //TODO: CHeck stride
        auto dims = buf.ndim();
        if (dims != 3 )
          throw std::runtime_error("Array must have three dimensions!");

        //Array comes to us in (y,x,z) form
        return new Array3D<float>((float*)buf.data(), buf.shape()[1], buf.shape()[0]);
      }))

      .def("size",      &Array3D<float>::size)
      .def("width",     &Array3D<float>::width)
      .def("height",    &Array3D<float>::height)
      .def("empty",     &Array3D<float>::empty)
      .def("noData",    &Array3D<float>::noData)

      //TODO: Simplify by casting to double in Python
      .def("setNoData", [](Array3D<float> &a, const float    ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const double   ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const int8_t   ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const int16_t  ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const int32_t  ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const int64_t  ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const uint8_t  ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const uint16_t ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const uint32_t ndval){ a.setNoData((float)ndval); })
      .def("setNoData", [](Array3D<float> &a, const uint64_t ndval){ a.setNoData((float)ndval); })

      .def_readwrite("geotransform", &Array3D<float>::geotransform)
      .def_readwrite("projection",   &Array3D<float>::projection)
      .def_readwrite("metadata",     &Array3D<float>::metadata)
      .def("copy", [](const Array3D<float> a){
        return a;
      })

      // .def_buffer([](Array3D<float> &arr) -> py::buffer_info {
      //   return py::buffer_info(
      //     arr.getData(),
      //     sizeof(T),
      //     py::format_descriptor<T>::format(),
      //     2,                                           //Dimensions
      //     {arr.height(), arr.width()},                 //Shape
      //     {sizeof(T) * arr.width(), sizeof(T)} //Stride (in bytes)
      //   );
      // })
      .def("__repr__",
        [=](const Array3D<float> &a) {
            return "<RichDEM 3D array: type=float, width="+std::to_string(a.width())+", height="+std::to_string(a.height())+", owned="+std::to_string(a.owned())+">";
        }
      )
      .def("__call__",
        [](Array3D<float> &a, const int x, const int y, const int n) -> float& {
          return a(x,y,n);
        }
      )
      .def("getIN",
        [](Array3D<float> &a, const int i, const int n) -> float& {
          return a.getIN(i,n);
        }
      );

  // Enregistrer la classe Array2D
  py::class_<Array2D<double>>(m, "Array2D")
    // Constructeurs
    .def(py::init<>())
    .def(py::init<Array2D<double>::xy_t, Array2D<double>::xy_t>(), "Create a 2D array with specified width and height")
    
    // Semblent fonctionner
    .def("size", &Array2D<double>::size, "Number of cells in the DEM")
    .def("width", &Array2D<double>::width, "Get the width of the array")
    .def("height", &Array2D<double>::height, "Get the height of the array")
    .def("viewXoff", &Array2D<double>::viewXoff, "X-Offset of this subregion of whatever raster we loaded from")
    .def("viewYoff", &Array2D<double>::viewYoff, "Y-Offset of this subregion of whatever raster we loaded from")
    .def("empty", &Array2D<double>::empty, "Returns TRUE if no data is present in RAM")
    .def("min", &Array2D<double>::min, "Finds the minimum value of the raster, ignoring NoData cells")
    .def("max", &Array2D<double>::max, "Finds the maximum value of the raster, ignoring NoData cells")
    .def("countval", &Array2D<double>::countval, "Counts the number of occurrences of a particular value in the raster. Can operate on NoData cells.")
    .def("iToxy", &Array2D<double>::iToxy, "Convert from index coordinates to x,y coordinates")
    .def("xyToI", &Array2D<double>::xyToI, "Convert from x,y coordinates to index coordinates")
    .def("nToI", &Array2D<double>::nToI, "Given a cell identified by an i-coordinate, return the i-coordinate of the neighbour identified by dx,dy")
    .def("isEquivalent", &Array2D<double>::operator==, "Determine if two rasters are equivalent based on dimensions, NoData value, and their data")
    .def("isNoData", [](Array2D<double>& self, Array2D<double>::xy_t x, Array2D<double>::xy_t y) -> bool {
        return self.isNoData(x, y);
    }, "Check if a cell is NoData using x, y coordinates")
    .def("isNoData", [](Array2D<double>& self, Array2D<double>::i_t i) -> bool {
        return self.isNoData(i);
    }, "Check if a cell is NoData using i coordinate")
    .def("isData", [](Array2D<double>& self, Array2D<double>::xy_t x, Array2D<double>::xy_t y) -> bool {
        return self.isData(x, y);
    }, "Check if a cell is Data using x, y coordinates")
    .def("isData", [](Array2D<double>& self, Array2D<double>::i_t i) -> bool {
        return self.isData(i);
    }, "Check if a cell is Data using i coordinate")
    .def("flipVert", &Array2D<double>::flipVert, "Flips the raster from top to bottom")
    .def("inGrid", &Array2D<double>::inGrid, "Test whether a cell lies within the boundaries of the raster")
    .def("isEdgeCell", [](Array2D<double>& self, Array2D<double>::xy_t x, Array2D<double>::xy_t y) -> bool {
        return self.isEdgeCell(x, y);
    }, "Test whether a cell lies on the boundary of the raster, using x, y coordinates")
    .def("isEdgeCell", [](Array2D<double>& self, Array2D<double>::i_t i) -> bool {
        return self.isEdgeCell(i);
    }, "Test whether a cell lies on the boundary of the raster, using i coordinate")
    .def("isTopLeft", &Array2D<double>::isTopLeft, "Determines whether an (x,y) pair is the top left of the DEM")
    .def("isTopRight", &Array2D<double>::isTopRight, "Determines whether an (x,y) pair is the top right of the DEM")
    .def("isBottomLeft", &Array2D<double>::isBottomLeft, "Determines whether an (x,y) pair is the bottom left of the DEM")
    .def("isBottomRight", &Array2D<double>::isBottomRight, "Determines whether an (x,y) pair is the bottom right of the DEM")
    .def("isTopRow", &Array2D<double>::isTopRow, "Determines whether an (x,y) pair is in the top row of the DEM")
    .def("isBottomRow", &Array2D<double>::isBottomRow, "Determines whether an (x,y) pair is in the bottom row of the DEM")
    .def("isLeftCol", &Array2D<double>::isLeftCol, "Determines whether an (x,y) pair is in the left column of the DEM")
    .def("isRightCol", &Array2D<double>::isRightCol, "Determines whether an (x,y) pair is in the right column of the DEM")
    .def("setAll", &Array2D<double>::setAll, "Sets all of the raster's cells to 'val'")
    .def("resize", [](Array2D<double>& self,  Array2D<double>::xy_t width0,  Array2D<double>::xy_t height0, const double& val0 = double()) {
        self.resize(width0, height0, val0);
    }, "Resize the raster. Note: this clears all the raster's data.")
    .def("resize", [](Array2D<double>& self, const Array2D<double>& other, const double& val = double()) {
        self.resize(other, val);
    }, "Resize a raster to copy another raster's dimensions. Copy properties.")
    .def("expand", &Array2D<double>::expand, "Makes a raster larger and retains the raster's old data, similar to resize. Note: Using this command requires RAM equal to the sum of the old raster and the new raster. The old raster is placed in the upper-left of the new raster.")
    .def("numDataCells", &Array2D<double>::numDataCells, "Returns the number of cells which are not NoData. May count them.")
    .def("getCellValue", [](const Array2D<double>& self, Array2D<double>::xy_t x, Array2D<double>::xy_t y) -> double {
        return self.operator()(x, y);
    }, "Return cell value based on x,y coordinates")
    .def("getCellValue", [](const Array2D<double>& self, Array2D<double>::i_t i) -> double {
        return self.operator()(i);
    }, "Return cell value based on i-coordinate")
    .def("topRow", &Array2D<double>::topRow, "Returns a copy of the top row of the raster")
    .def("bottomRow", &Array2D<double>::bottomRow, "Returns a copy of the bottom row of the raster")
    .def("leftColumn", &Array2D<double>::leftColumn, "Returns a copy of the left column of the raster")
    .def("rightColumn", &Array2D<double>::rightColumn, "Returns a copy of the right column of the raster")
    .def("setRow", &Array2D<double>::setRow, "Sets an entire row of a raster to a given value.")
    .def("setCol", &Array2D<double>::setCol, "Sets an entire column of a raster to a given value.")
    .def("setEdges", &Array2D<double>::setEdges, "Sets the edges of the array to a given value.")
    .def("getRowData", &Array2D<double>::getRowData, "Returns a copy of an arbitrary row of the raster")
    .def("getColData", &Array2D<double>::getColData, "Returns a copy of an arbitrary column of the raster")
    .def("printAll", [](const Array2D<double>& self) {
        self.printAll();
    }, "Prints the entire array with default parameters")
    .def("printAll", [](const Array2D<double>& self, const std::string& msg) {
        self.printAll(msg);
    }, "Prints the entire array with a custom message")
    .def("printAll", [](const Array2D<double>& self, const std::string& msg, int fwidth) {
        self.printAll(msg, fwidth);
    }, "Prints the entire array with a custom message and field width")
    .def("printAll", [](const Array2D<double>& self, const std::string& msg, int fwidth, int precision) {
        self.printAll(msg, fwidth, precision);
    }, "Prints the entire array with a custom message, field width, and precision")
    .def("printAllIndices", &Array2D<double>::printAllIndices, "Prints the flat indices of the entire array")
    
    // Ne renvoie pas d'erreur, mais renvoie "Segmentation fault" lorsqu'on cherche à accéder à une valeur
    // Par exemple en faisant `array.min()
    .def("clear", &Array2D<double>::clear, "Clear the array")

    // Ne renvoie pas d'erreur. Si je remplace le max par une autre valeur et que je fais array.max(), j'obtiens
    // bien la nouvelle valeur, mais lorsque je fais array.printAll() rien n'a été remplacé
    .def("replace", &Array2D<double>::replace, "Replace one cell value with another throughout the raster. Can operate on NoData cells.")
    
    // A tester
    .def("loadData", &Array2D<double>::loadData, "Load data into the array")
    .def("saveToCache", [](Array2D<double>& self, const std::string& cache_filename) {
        self.saveToCache(cache_filename);
    }, "Save the array to cache")
    .def("dumpData", &Array2D<double>::dumpData, "Dump the array data")
    .def("setNoData", &Array2D<double>::setNoData, "Set the NoData value")
    .def("noData", &Array2D<double>::noData, "Get the NoData value")
    .def("getN", &Array2D<double>::getN, "Given a cell identified by an i-coordinate, return the i-coordinate of the neighbour identified by n")
    .def("nshift", &Array2D<double>::nshift, "Return the offset of the neighbour cell identified by n")
    .def("flipHorz", &Array2D<double>::flipHorz, "Flips the raster from side-to-side")
    .def("transpose", &Array2D<double>::transpose, "Flips the raster about its diagonal axis, like a matrix tranpose.")
    .def("templateCopy", [](Array2D<double>& self, const Array2D<double>& other) {
        self.templateCopy(other);
    }, "Copies the geotransform, projection, and basename of another raster")
    .def("printAllFlows", &Array2D<double>::printAllFlows, "Prints the entire array as flow directions")
    .def("printBlockIndices", &Array2D<double>::printBlockIndices, "Prints a square of cells centered at x,y indicating the index of each")
    .def("getCellArea", &Array2D<double>::getCellArea, "Get the area of an individual cell in square projection units")
    .def("getCellLengthX", &Array2D<double>::getCellLengthX, "Get the length of a cell along the raster's horizontal axis")
    .def("getCellLengthY", &Array2D<double>::getCellLengthY, "Get the length of a cell along the raster's vertical axis")
    .def("scale", &Array2D<double>::scale, "Multiplies the entire array by a scalar")
    .def("owned", &Array2D<double>::owned)

    .def("__getitem__", [](Array2D<double>& self, std::pair<Array2D<double>::xy_t, Array2D<double>::xy_t> indices) -> double& {
        return self(indices.first, indices.second);
    }, "Get the value at the specified indices")
    .def("__setitem__", [](Array2D<double>& self, std::pair<Array2D<double>::xy_t, Array2D<double>::xy_t> indices, double value) {
        self(indices.first, indices.second) = value;
    }, "Set the value at the specified indices");

  // m.def("generate_perlin_terrain", &richdem::generate_perlin_terrain, "Generate random terrain using perlin noise", py::arg("array"), py::arg("seed"));

  // Définir la fonction generate_perlin_terrain avec des lambdas pour la conversion de types
  // La version qui prend un Array2D et une seed
  m.def("generate_perlin_terrain_array", [](Array2D<double>& array, uint32_t seed) {
      return richdem::generate_perlin_terrain(array, seed);
  }, "Generate random terrain using perlin noise", py::arg("array"), py::arg("seed"));

  // Définir la fonction generate_perlin_terrain avec des lambdas pour la conversion de types
  // La version qui prend une size et une seed
  m.def("generate_perlin_terrain", [](uint32_t size, uint32_t seed) {
      return richdem::generate_perlin_terrain(size, seed);
  }, "Generate random terrain using perlin noise", py::arg("size"), py::arg("seed"));

  py::module_ dephier_module = m.def_submodule("depression_hierarchy", "Depression Hierarchies");

  dephier_module.attr("NO_PARENT") = &dephier::NO_PARENT;
  dephier_module.attr("NO_VALUE") = &dephier::NO_VALUE;
  dephier_module.attr("NO_DEP") = &dephier::NO_DEP;
  dephier_module.attr("OCEAN") = &dephier::OCEAN;

  // Depression Hierarchy
  py::class_<dephier::Depression<double>>(dephier_module, "Depression")
    .def(py::init<>())
    .def_readwrite("pit_cell",        &dephier::Depression<double>::pit_cell, "Flat index of the pit cell, the lowest cell in the depression. If more than one cell shares this lowest elevation, then one is arbitrarily chosen.")
    .def_readwrite("out_cell",        &dephier::Depression<double>::out_cell, "Flat index of the outlet cell. If there is more than one outlet cell at this cell's elevation, then one is arbitrarily chosen.")
    .def_readwrite("parent",          &dephier::Depression<double>::parent, "Parent depression. If both this depression and its neighbour fill up, this parent depression is the one which will contain the overflow.")
    .def_readwrite("odep",            &dephier::Depression<double>::odep, "Outlet depression. The metadepression into which this one overflows. Usually its neighbour depression, but sometimes the ocean.")
    .def_readwrite("geolink",         &dephier::Depression<double>::geolink, "When a metadepression overflows it does so into the metadepression indicated by `odep`. However, odep must flood from the bottom up. Therefore, we keep track of the `geolink`, which indicates what leaf depression the overflow is initially routed into.")
    .def_readwrite("pit_elev",        &dephier::Depression<double>::pit_elev, "Elevation of the pit cell. Since the pit cell has the lowest elevation of any cell in the depression, we initialize this to infinity.")
    .def_readwrite("out_elev",        &dephier::Depression<double>::out_elev, "Elevation of the outlet cell. Since the outlet cell has the lowest elevation of any path leading from a depression, we initialize this to infinity.")
    .def_readwrite("lchild",          &dephier::Depression<double>::lchild, "The depressions form a binary tree. Each depression has two child depressions: one left and one right.")
    .def_readwrite("rchild",          &dephier::Depression<double>::rchild, "The depressions form a binary tree. Each depression has two child depressions: one left and one right.")
    .def_readwrite("ocean_parent",    &dephier::Depression<double>::ocean_parent, "Indicates whether the parent link is to either the ocean or a depression that links to the ocean.")
    .def_readwrite("ocean_linked",    &dephier::Depression<double>::ocean_linked, "Indicates depressions which link to the ocean through this depression, but are not subdepressions. That is, these ocean-linked depressions may be at the top of high cliffs and spilling into this depression.")
    .def_readwrite("dep_label",       &dephier::Depression<double>::dep_label, "The label of the depression, for calling it up again.")
    .def_readwrite("cell_count",      &dephier::Depression<double>::cell_count, "Number of cells contained within the depression and its children.")
    .def_readwrite("dep_vol",         &dephier::Depression<double>::dep_vol, "Volume of the depression and its children. Used in the Water Level Equation (see below).")
    .def_readwrite("water_vol",       &dephier::Depression<double>::water_vol, "Water currently contained within the depression. Used in the Water Level Equation (see below).")
    .def_readwrite("total_elevation", &dephier::Depression<double>::total_elevation, "Total elevation of cells contained with the depression and its children.")
  ; //Ends the class definition above

  dephier_module.def("get_depression_hierarchy", &dephier::GetDepressionHierarchy<double, Topology::D8>, "Calculate the hierarchy of depressions. Takes as input a digital elevation model and a set of labels. The labels should have `OCEAN` for cells");
  dephier_module.def(
    "fill_spill_merge",
    &dephier::FillSpillMerge<double, double>,
    "Perform Fill-Spill-Merge on a given landscape and its associated depression hierarchy and water table depths",
    py::arg("topo"),
    py::arg("labels"),
    py::arg("flowdirs"),
    py::arg("deps"),
    py::arg("wtd")
  );
}
