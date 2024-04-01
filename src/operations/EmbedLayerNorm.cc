#include "EmbedLayerNorm.h"
#include "../Model.h"
#include "../Tensor.h"

EmbedLayerNorm::EmbedLayerNorm(SimulationConfig config, Model* model, onnx::NodeProto& node_proto)
    : Operation(config, model, node_proto) {
  _input_shape = get_input(0)->get_dims();
  _weight_shape = get_input(2)->get_dims();

  assert(_input_shape.size()==2);
  _output_shape.push_back(_input_shape.at(0));
  _output_shape.push_back(_input_shape.at(1)); 
  _output_shape.push_back(_weight_shape.at(1)); 
  spdlog::trace("output_shape : {}", _output_shape);

  Tensor* embed_output = _model->find_tensor(node_proto.output(0));
  if (embed_output == nullptr) {
    std::unique_ptr<Tensor> output_tensor = std::make_unique<Tensor>(
        _id, node_proto.output(0), _output_shape, _config.precision, false);
    _outputs.push_back(output_tensor.get()->get_id());
    _model->add_tensor(std::move(output_tensor));
  } else {
    embed_output->redefine_tensor(_id, _output_shape);
  }

  /* mask */
  Tensor* mask_output = _model->find_tensor(node_proto.output(1));
  if (mask_output == nullptr) {
    std::unique_ptr<Tensor> output_tensor = std::make_unique<Tensor>(
        _id, node_proto.output(1), _output_shape, _config.precision, false);
    _outputs.push_back(output_tensor.get()->get_id());
    _model->add_tensor(std::move(output_tensor));
  } else {
    mask_output->redefine_tensor(_id, _output_shape);
  }
  if (node_proto.output().size()==3) {
    Tensor* embed_sum = _model->find_tensor(node_proto.output(2));
    if (embed_sum == nullptr) {
      std::unique_ptr<Tensor> output_tensor = std::make_unique<Tensor>(
          _id, node_proto.output(2), _output_shape, _config.precision, false);
      _outputs.push_back(output_tensor.get()->get_id());
      _model->add_tensor(std::move(output_tensor));
    } else {
      embed_sum->redefine_tensor(_id, _output_shape);
    }
  }
}

void EmbedLayerNorm::initialize_tiles(MappingTable& mapping_table) {
  std::unique_ptr<Tile> tile = std::make_unique<Tile>(Tile{
                        .status = Tile::Status::INITIALIZED,
                        .optype="EmbedLayerNorm",
                        .layer_id=_id,
                        .skip=true});
  _tiles.push_back(std::move(tile));
}

void EmbedLayerNorm::initialize_instructions(Tile* tile, Mapping mapping) {
}