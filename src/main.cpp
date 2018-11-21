/*
#include <util/Logger.hpp>

#include "Program.hpp"

int main( int argc, const char* argv[] )
{
    util::Logger::setName( "Main log", "log.txt" );

    try
    {
        Program program;
        program.run();
    }
    catch ( std::exception& e )
    {
        util::log( "Unhandled exception: $\n", e.what() );
    }
}
//*/

//*
#include <imgui.h>
#include <imgui-sfml.h>
#include <SFML/Graphics.hpp>

#include "NodeGraph.hpp"

int main()
{
    sf::RenderWindow window;
    window.create( sf::VideoMode( 800, 600 ), "Stardew Editor 2" );
    window.setVerticalSyncEnabled( true );
    ImGui::SFML::Init( window );

    NodeGraph::Graph nodes;
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Start", { {}, { { NodeGraph::ConnectionType::Sequence, "" } }, false } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Nop", { { { NodeGraph::ConnectionType::Sequence, "" } }, { { NodeGraph::ConnectionType::Sequence, "" } } } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Print", { { { NodeGraph::ConnectionType::Sequence, "" }, { NodeGraph::ConnectionType::String, "String" } }, { { NodeGraph::ConnectionType::Sequence, "" } } } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Concat", { { { NodeGraph::ConnectionType::String, "String 1" }, { NodeGraph::ConnectionType::String, "String 2" } }, { { NodeGraph::ConnectionType::String, "Output" } } } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Int ToString", { { { NodeGraph::ConnectionType::Int, "Input" } }, { { NodeGraph::ConnectionType::String, "Output" } } } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Float ToString", { { { NodeGraph::ConnectionType::Float, "Input" } }, { { NodeGraph::ConnectionType::String, "Output" } } } ) );
    nodes.types.insert( std::make_pair< std::string, NodeGraph::NodeType >( "Split Vec2", { { { NodeGraph::ConnectionType::Vector2, "Input" } }, { { NodeGraph::ConnectionType::Int, "X" }, { NodeGraph::ConnectionType::Int, "Y" } } } ) );

    {
        auto startNode = std::make_unique< NodeGraph::Node >();
        startNode->type = "Start";
        startNode->outputs.resize( 1 );

        auto floatNode = std::make_unique< NodeGraph::Node >();
        floatNode->type = "Float ToString";
        floatNode->inputs.resize( 1 );
        floatNode->inputs[ 0 ] = 3.14f;
        floatNode->outputs.resize( 1 );
        floatNode->position = ImVec2( 100, 150 );

        auto printNode = std::make_unique< NodeGraph::Node >();
        printNode->type = "Print";
        printNode->inputs.resize( 2 );
        printNode->outputs.resize( 1 );
        printNode->position = ImVec2( 400, 0 );

        NodeGraph::Connection conn1o;
        conn1o.other = startNode.get();
        conn1o.index = 0;
        printNode->inputs[ 0 ] = conn1o;
        NodeGraph::Connection conn1i;
        conn1i.other = printNode.get();
        conn1i.index = 0;
        startNode->outputs[ 0 ] = conn1i;

        NodeGraph::Connection conn2o;
        conn2o.other = floatNode.get();
        conn2o.index = 0;
        printNode->inputs[ 1 ] = conn2o;
        NodeGraph::Connection conn2i;
        conn2i.other = printNode.get();
        conn2i.index = 1;
        floatNode->outputs[ 0 ] = conn2i;
        floatNode->selected = true;

        nodes.nodes.push_back( std::move( startNode ) );
        nodes.nodes.push_back( std::move( floatNode ) );
        nodes.nodes.push_back( std::move( printNode ) );
    }

    sf::Clock clock;
    bool isRunning = true;
    while ( isRunning )
    {
        sf::Event event;
        while ( window.pollEvent( event ) )
        {
            ImGui::SFML::ProcessEvent( event );
            if ( event.type == sf::Event::Closed )
                isRunning = false;
            else if ( event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Delete )
                nodes.deletePressed();
        }
        ImGui::SFML::Update( window, clock.restart() );
        nodes.update();
        ImGui::EndFrame();

        window.clear( sf::Color::White );
        ImGui::SFML::Render( window );
        window.display();
    }
}
//*/
