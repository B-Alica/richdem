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

// In order to not register the same type multiple times
std::unordered_set<std::string> registered_types;

template <typename T>
void CustomTemplatedFunctionsWrapper(py::module &m, const std::string &type_name) {
    // Check if type is already register
    if (registered_types.find(type_name) == registered_types.end()) {
        registered_types.insert(type_name);

        TemplatedFunctionsWrapper<T>(m, type_name);
    }
}

template <typename T>
void CustomTemplatedArrayWrapper(py::module &m, const std::string &type_name) {
    // Check if type is already register
    if (registered_types.find(type_name) == registered_types.end()) {
        registered_types.insert(type_name);

        TemplatedArrayWrapper<T>(m, type_name);
    }
}

// Template function to expose Array2D
template <typename T>
void exposeArray2D(py::module &m, const std::string &type_name) {
    using ClassName = Array2D<T>;

    py::class_<ClassName>(m, ("Array2D_" + type_name).c_str())
        // Constructors
        .def(py::init<>())
        .def(py::init<typename ClassName::xy_t, typename ClassName::xy_t>(), "Create a 2D array with specified width and height")
        .def(py::init([](py::array_t<T> array) {
            // Get the info of the numpy array
            py::buffer_info buf = array.request();
            if (buf.ndim != 2) {
                throw std::runtime_error("Number of dimensions must be 2");
            }

            // Get the array shape
            typename ClassName::xy_t width = buf.shape[1];
            typename ClassName::xy_t height = buf.shape[0];

            // Create a Array2D
            return new ClassName(static_cast<T*>(buf.ptr), width, height);
        }), "Create a 2D array from a NumPy array")

        // Methods
        .def("size", &ClassName::size, "Number of cells in the DEM")
        .def("width", &ClassName::width, "Get the width of the array")
        .def("height", &ClassName::height, "Get the height of the array")
        .def("empty", &ClassName::empty, "Returns TRUE if no data is present in RAM")
        .def("min", &ClassName::min, "Finds the minimum value of the raster, ignoring NoData cells")
        .def("max", &ClassName::max, "Finds the maximum value of the raster, ignoring NoData cells")
        .def("noData", &ClassName::noData, "Get the no_data value")
        .def("setNoData", &ClassName::setNoData, "Set the NoData value")

        // Properity to expose the Array2D geotransform attribute
        // Getter then setter
        .def_property("geotransform",
            [](ClassName &self) {
                return py::array_t<double>(self.geotransform.size(), self.geotransform.data());
            },
            [](ClassName &self, py::array_t<double> array) {
                py::buffer_info buf = array.request();

                // Obtenez un pointeur vers les données du tableau
                double* ptr = static_cast<double*>(buf.ptr);

                // Assignez tous les éléments au vecteur geotransform
                self.geotransform.assign(ptr, ptr + buf.size);
            })

        .def("__getitem__", [](ClassName &self, std::pair<typename ClassName::xy_t, typename ClassName::xy_t> indices) -> T& {
            return self(indices.first, indices.second);
        }, "Get the value at the specified indices")
        .def("__setitem__", [](ClassName &self, std::pair<typename ClassName::xy_t, typename ClassName::xy_t> indices, T value) {
            self(indices.first, indices.second) = value;
        }, "Set the value at the specified indices");
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

  // Expose Array2D for different types
  exposeArray2D<int8_t>(m, "int8_t");
  exposeArray2D<int16_t>(m, "int16_t");
  exposeArray2D<int32_t>(m, "int32_t");
  exposeArray2D<int64_t>(m, "int64_t");
  exposeArray2D<uint8_t>(m, "uint8_t");
  exposeArray2D<uint16_t>(m, "uint16_t");
  exposeArray2D<uint32_t>(m, "uint32_t");
  exposeArray2D<uint64_t>(m, "uint64_t");
  exposeArray2D<float>(m, "float");
  exposeArray2D<double>(m, "double");

  // Expose generate_perlin_terrain using and array and a seed
  m.def("generate_perlin_terrain", [](Array2D<double>& array, uint32_t seed) {
      return richdem::generate_perlin_terrain(array, seed);
  }, "Generate random terrain using perlin noise", py::arg("array"), py::arg("seed"));

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
