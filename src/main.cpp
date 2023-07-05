// Oleg Kotov

/*

    Example of creating a simple game
    The idea was taken from Pinterest
    There may be bugs in the project
    You can use the code as a reference for your own projects
    Enjoy
    
    https://ru.pinterest.com/pin/499618152419223621/

*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ArduinoNvs.h>
#include <esp_random.h>
#include <esp_system.h>
#include <vector>

#include "clock.h"
#include "vector2.h"
#include "mathutils.h"
#include "button.h"

// font
#include "montserrat32.h"

// game state
enum class GameState : uint8_t
{
    MainMenu,
    Gameplay,
    GameOver
};

GameState gameState = GameState::MainMenu;

// button
constexpr int button_x_pin = 12;
Button button_x( button_x_pin );

// display and canvas

TFT_eSPI display = TFT_eSPI( 240, 240 );
TFT_eSprite canvas = TFT_eSprite( &display );

const uint8_t canvasBitsPerPixel = 16;

// init later
uint16_t canvasWidth = 0;
uint16_t canvasHeight = 0;

uint16_t canvasCenterX = 0;
uint16_t canvasCenterY = 0;

// game clock
Clock gameClock;

// colors

uint16_t backgroundColor  = display.color565( 245, 76, 57 );
uint16_t blackColor = display.color565( 44, 16, 15 );
uint16_t whiteColor = display.color565( 245, 245, 245 );

// sprites

TFT_eSprite playerSprite = TFT_eSprite( &display );
TFT_eSprite tailSprite = TFT_eSprite( &display );
TFT_eSprite squareSprite = TFT_eSprite( &display );
TFT_eSprite wallSprite = TFT_eSprite( &display );

// stats
int bestScore = 0;
uint16_t pointCount = 0;
String pointCountLabel = "0";

// ring
float ringRadius = 80.0f;
float ringBorderWidth = 7.0f;
uint16_t ringBackgroundColor = backgroundColor;
uint16_t ringBorderColor = blackColor;

// player

float playerRadius = 7.0f;
float playerOffset = 2.0f;

float playerInsideDistance = ringRadius - ( ( ringBorderWidth * 0.5f ) + playerRadius + playerOffset );
float playerOutsideDistance = ringRadius + ( ( ringBorderWidth * 0.5f ) + playerRadius + playerOffset );

bool isPlayerInsideRing = false;
float playerDistance = playerOutsideDistance;
float playerAngle = 90.0f;
Vector2 playerPosition;
float playerSpeed = 80.0f; // 90
uint16_t playerColor = whiteColor;

// square

Vector2 squarePosition;
float squareHalfSize = 4.5f;
uint16_t squareColor = whiteColor;
float squareRotation = 0.0f;
float squareRotationSpeed = 110.0f;
float squareSpawnOffset = 25.0f; // 18

// dash animation

float dashAnimTotalTime = 0.15f;
float dashAnimCurrentTime = dashAnimTotalTime;

// wall animation

float wallAnimTotalTime = 0.12f;
float wallMoveChance = 1.0f / 3.0f;

// wall

float wallHalfWidth = 12.0f;
float wallHalfHeight = 4.35f;

float wallInsideDistance = ringRadius - ( ( ringBorderWidth * 0.5f ) + wallHalfWidth - 2 );
float wallOutsideDistance = ringRadius + ( ( ringBorderWidth * 0.5f ) + wallHalfWidth - 2 );

struct Wall
{
    bool spawned = false;
    bool isInsideRing = false;
    float distance = wallOutsideDistance;
    float angle = 0.0f;
    Vector2 position;
    float animCurrentTime = wallAnimTotalTime;
};

const uint8_t wallCount = 5;
Wall walls[wallCount];

// spawn wall

uint8_t spawnWallNeed = 6;
uint8_t spawnWallCounter = 0;
uint8_t spawnedWallCount = 0;
float minSpawnAngle = 90.0f;

// tail effect

float tailRadius = playerRadius * 0.45f;
const uint8_t tailLength = 4;
float tailSpacing = 7.5f;

float tailSpawnInterval = -1;
float tailSpawnTimer = 0;

struct TailPart
{
    Vector2 position;
};

std::vector<TailPart> tail;

// calculate canvas size
uint16_t canvasSize = ringRadius + ( ( ringBorderWidth * 0.5f ) + ( wallHalfWidth * 2.0f ) );

void createPlayerSprite( TFT_eSprite& sprite )
{
    sprite.setColorDepth( canvasBitsPerPixel );
    sprite.createSprite( playerRadius * 2, playerRadius * 2 );

    sprite.setPivot( playerRadius, playerRadius );

    sprite.fillSprite( TFT_TRANSPARENT );
    sprite.fillSmoothCircle( playerRadius, playerRadius, playerRadius - 1, playerColor, backgroundColor );
}

void createTailSprite( TFT_eSprite& sprite )
{
    sprite.setColorDepth( canvasBitsPerPixel );
    sprite.createSprite( tailRadius * 2, tailRadius * 2 );

    sprite.setPivot( tailRadius, tailRadius );

    sprite.fillSprite( TFT_TRANSPARENT );
    sprite.fillSmoothCircle( tailRadius, tailRadius, tailRadius - 1, playerColor, backgroundColor );
}

void createSquareSprite( TFT_eSprite& sprite )
{
    sprite.setColorDepth( canvasBitsPerPixel );
    sprite.createSprite( squareHalfSize * 2.0f, squareHalfSize * 2.0f );

    sprite.setPivot( squareHalfSize, squareHalfSize );

    sprite.fillSprite( TFT_TRANSPARENT );
    sprite.fillRect( 0.0f, 0.0f, squareHalfSize * 2.0f, squareHalfSize * 2.0f , squareColor );
}

void createWallSprite( TFT_eSprite& sprite )
{
    sprite.setColorDepth( canvasBitsPerPixel );
    sprite.createSprite( wallHalfWidth * 2.0f, wallHalfHeight * 2.0f );

    sprite.setPivot( wallHalfWidth, wallHalfHeight );

    sprite.fillSprite( TFT_TRANSPARENT );
    sprite.fillRect( 0.0f, 0.0f, wallHalfWidth * 2.0f, wallHalfHeight * 2.0f , blackColor );
}

void printFreeHeap()
{
    printf( "- free heap: %u bytes\n", ESP.getFreeHeap() );
}

void save()
{
    NVS.setInt( "bestScore", bestScore );
}

void load()
{
    bestScore = NVS.getInt( "bestScore" );
}

void addPoint()
{
    pointCount++;
    pointCountLabel = pointCount;
}

void gameOver()
{
    gameState = GameState::GameOver;

    if ( pointCount > bestScore )
    {
        bestScore = pointCount;
        save();
    }
}

void spawnWall( uint8_t wallIndex )
{
    Wall& wall = walls[wallIndex];

    wall.isInsideRing = getRandomBool();

    wall.distance = ( wall.isInsideRing ) ? wallInsideDistance : wallOutsideDistance;

    wall.position.x = cosf( -wall.angle * Deg2Rad ) * wall.distance;
    wall.position.y = sinf( -wall.angle * Deg2Rad ) * wall.distance;

    wall.spawned = true;

    spawnedWallCount++;
    spawnWallCounter = 0;
}

void initWallAngles()
{
    float angleStep = 360.0f / wallCount;
    float angle = 0.0f;

    for ( uint8_t i = 0; i < wallCount; ++i )
    {
        Wall& wall = walls[i];

        wall.angle = angle;
        angle += angleStep;
    }
}

void updateTailSpacing()
{
    float r = playerDistance;
    float w = playerSpeed * Deg2Rad;

    float playerLinearVelocity = r * w;

    tailSpawnInterval = tailSpacing / playerLinearVelocity;
}

void updateTail( float deltaTime )
{
    tailSpawnTimer -= deltaTime;
    if ( tailSpawnTimer > 0.0f ) return;

    TailPart newTailPart;
    newTailPart.position = playerPosition;

    tail.push_back( newTailPart );

    if ( tail.size() > tailLength )
    {
        tail.erase( tail.begin() );
    }

    tailSpawnTimer = tailSpawnInterval;
}

void setSquareRandomPosition()
{
    int spawnSector = getRandomNumberInRange( 0, wallCount - 1 );
    // bool isPositiveOffset = getRandomBool();
    bool shouldSpawnInsideRing = getRandomBool();

    // float offset = ( isPositiveOffset ) ? squareSpawnOffset : -squareSpawnOffset;
    float offset = squareSpawnOffset;
    float angleRadians = ( walls[spawnSector].angle + offset ) * Deg2Rad;
    float distance = ( shouldSpawnInsideRing ) ? playerInsideDistance : playerOutsideDistance;

    squarePosition.x = cosf( -angleRadians ) * distance;
    squarePosition.y = sinf( -angleRadians ) * distance;    
}

void updatePlayerAngle( float deltaTime )
{
    playerAngle += playerSpeed * deltaTime;
    if ( playerAngle > 360.0f ) playerAngle -= 360.0f;
}

void updatePlayerDistance( float deltaTime )
{
    if ( dashAnimCurrentTime >= dashAnimTotalTime ) return;

    dashAnimCurrentTime += deltaTime;
    if ( dashAnimCurrentTime > dashAnimTotalTime ) dashAnimCurrentTime = dashAnimTotalTime;

    float t = dashAnimCurrentTime / dashAnimTotalTime;
    if ( !isPlayerInsideRing ) t = 1.0f - t;

    playerDistance = lerp( playerOutsideDistance, playerInsideDistance, t );

    updateTailSpacing();
}

void updatePlayerPosition()
{
    playerPosition.x = cosf( -playerAngle * Deg2Rad ) * playerDistance;
    playerPosition.y = sinf( -playerAngle * Deg2Rad ) * playerDistance;
}

void updateSquareRotation( float deltaTime )
{
    squareRotation += squareRotationSpeed * deltaTime;
    if ( squareRotation > 360.0f ) squareRotation -= 360.0f;
}

void updateWallDistance( float deltaTime )
{
    for ( int8_t i = 0; i < wallCount; ++i )
    {
        Wall& wall = walls[i];

        if ( wall.animCurrentTime == wallAnimTotalTime ) continue;

        wall.animCurrentTime += deltaTime;
        if ( wall.animCurrentTime > wallAnimTotalTime ) wall.animCurrentTime = wallAnimTotalTime;

        float t = wall.animCurrentTime / wallAnimTotalTime;
        if ( !wall.isInsideRing ) t = 1.0f - t;

        wall.distance = lerp( wallOutsideDistance, wallInsideDistance, t );
    }
}

void updateWallPosition()
{
    for ( int8_t i = 0; i < wallCount; ++i )
    {
        Wall& wall = walls[i];

        wall.position.x = cosf( -wall.angle * Deg2Rad ) * wall.distance;
        wall.position.y = sinf( -wall.angle * Deg2Rad ) * wall.distance;
    }
}

bool needSpawnWall()
{
    if ( spawnedWallCount == wallCount ) return false;
    if ( spawnWallCounter < spawnWallNeed ) return false;
    return true;
}

int8_t canSpawnRandomWall()
{
    while ( true )
    {
        uint8_t wallIndex = getRandomNumberInRange( 0, wallCount - 1 );
        Wall& wall = walls[wallIndex];

        if ( wall.spawned ) continue;

        // check distance between player and wall spawn
        float angleDiff = fabsf( playerAngle - wall.angle );
        if ( angleDiff > 180.0f ) angleDiff = 360.0f - angleDiff;

        return ( angleDiff < minSpawnAngle ) ? -1 : wallIndex;
    }
}

void moveRandomWalls()
{
    for ( int8_t i = 0; i < wallCount; ++i )
    {
        Wall& wall = walls[i];

        if ( !wall.spawned ) continue;
        if ( getRandomNumber() > wallMoveChance ) continue;

        wall.isInsideRing = !wall.isInsideRing;
        wall.animCurrentTime = 0.0f;
    }
}

void onPlayerWithSquareCollision()
{
    addPoint();
    setSquareRandomPosition();

    spawnWallCounter++;

    if ( needSpawnWall() )
    {
        int8_t wallIndex = canSpawnRandomWall();

        if ( wallIndex >= 0 )
        {
            spawnWall( wallIndex );
            return;
        }
    }
    
    moveRandomWalls();
}

void onPlayerWithWallCollision()
{
    gameOver();
}

bool checkPlayerWithSquareCollision()
{
    float dx = playerPosition.x - squarePosition.x;
    float dy = playerPosition.y - squarePosition.y;
    float distance = sqrtf( dx * dx + dy * dy );

    return distance < playerRadius + squareHalfSize;
}

bool checkPlayerWithWallCollision( uint8_t index )
{
    Wall& wall = walls[index];

    float dx = playerPosition.x - wall.position.x;
    float dy = playerPosition.y - wall.position.y;
    float cosAngle = cosf( wall.angle * Deg2Rad );
    float sinAngle = sinf( wall.angle * Deg2Rad );
    float xRotated = dx * cosAngle + dy * sinAngle;
    float yRotated = -dx * sinAngle + dy * cosAngle;

    float rectRight = wallHalfWidth;
    float rectTop = wallHalfHeight;
    float circleDistanceX = fabsf( xRotated ) - rectRight;
    float circleDistanceY = fabsf( yRotated ) - rectTop;

    if ( circleDistanceX > playerRadius || circleDistanceY > playerRadius ) return false;
    if ( circleDistanceX <= 0 || circleDistanceY <= 0 ) return true;

    float cornerDistanceSq = ( circleDistanceX * circleDistanceX ) + ( circleDistanceY * circleDistanceY );
    return cornerDistanceSq <= ( playerRadius * playerRadius );
}

bool checkPlayerWithWallsCollision()
{
    for ( uint8_t i = 0; i < wallCount; ++i )
    {
        if ( !walls[i].spawned ) continue;
        if ( checkPlayerWithWallCollision( i ) ) return true;
    }

    return false;
}

void drawRing()
{
    canvas.fillSmoothCircle( canvasCenterX, canvasCenterY, ringRadius + ( ringBorderWidth * 0.5f ), ringBorderColor );
    canvas.fillSmoothCircle( canvasCenterX, canvasCenterY, ringRadius - ( ringBorderWidth * 0.5f ), ringBackgroundColor, ringBorderColor );
}

void drawPointCountText()
{
    canvas.drawString( pointCountLabel, canvasCenterX, canvasCenterY );
}

void drawFramerateCounter( float deltaTime )
{
    int16_t framerate = 1.0f / deltaTime;

    String text;
    text.concat( framerate );

    canvas.drawString( text, canvasCenterX, canvasCenterY );
}

void drawPlayerSprite()
{
    playerSprite.pushToSprite( &canvas, canvasCenterX - playerRadius + playerPosition.x, canvasCenterY - playerRadius + playerPosition.y, TFT_TRANSPARENT );
}

void drawSquareSprite()
{
    canvas.setPivot( canvasCenterX + squarePosition.x, canvasCenterY + squarePosition.y );
    squareSprite.pushRotated( &canvas, -squareRotation, TFT_TRANSPARENT );
}

void drawWallSprite( uint8_t index )
{
    Wall& wall = walls[index];

    canvas.setPivot( canvasCenterX + wall.position.x, canvasCenterY + wall.position.y );
    wallSprite.pushRotated( &canvas, -wall.angle, TFT_TRANSPARENT );
}

void drawWallSprites()
{
    for ( uint8_t i = 0; i < wallCount; ++i )
    {
        if ( walls[i].spawned ) drawWallSprite( i );
    }
}

void drawTail()
{
    for ( uint8_t i = 0; i < tail.size(); ++i )
    {
        TailPart& tailPart = tail[i];

        tailSprite.pushToSprite( &canvas, canvasCenterX - tailRadius + tailPart.position.x, canvasCenterY - tailRadius + tailPart.position.y, TFT_TRANSPARENT );
    }
}

void fillCanvas( uint16_t color )
{
    canvas.fillSprite( color );
}

void drawCanvas()
{
    canvas.pushSprite( ( display.width() - canvasWidth ) * 0.5f, ( display.height() - canvasHeight ) * 0.5f );
}

void dash()
{
    isPlayerInsideRing = !isPlayerInsideRing;
    dashAnimCurrentTime = 0.0f;
}

void drawMainMenu( float deltaTime )
{
    String text;
    text.concat( "BEST " );
    text.concat( bestScore );

    fillCanvas( ringBackgroundColor );
    canvas.drawString( text, canvasCenterX, canvasCenterY );
    drawCanvas();
}

void updateMainMenu( float deltaTime )
{
    if ( button_x.click() )
    {
        gameState = GameState::Gameplay;
    }

    drawMainMenu( deltaTime );
}

void drawGameplay()
{
    fillCanvas( ringBackgroundColor );

    drawRing();
    drawWallSprites();
    drawPointCountText();
    // drawFramerateCounter( deltaTime );
    drawSquareSprite();
    drawTail();
    drawPlayerSprite();
    
    drawCanvas();
}

void updateGameplay( float deltaTime )
{
    if ( button_x.click() ) dash();

    updatePlayerAngle( deltaTime );
    updatePlayerDistance( deltaTime );
    updatePlayerPosition();

    updateTail( deltaTime );

    updateWallDistance( deltaTime );
    updateWallPosition();

    updateSquareRotation( deltaTime );

    if ( checkPlayerWithSquareCollision() )
    {
        onPlayerWithSquareCollision();
    }

    if ( checkPlayerWithWallsCollision() )
    {
        onPlayerWithWallCollision();
    }

    drawGameplay();
}

void drawGameOver()
{
    String scoreText;
    scoreText.concat( "SCORE " );
    scoreText.concat( pointCount );

    String bestText;
    bestText.concat( "BEST " );
    bestText.concat( bestScore );

    uint8_t textOffset = 18;

    fillCanvas( ringBackgroundColor );
    canvas.drawString( scoreText, canvasCenterX, canvasCenterY - textOffset );
    canvas.drawString( bestText, canvasCenterX, canvasCenterY + textOffset );
    drawCanvas();
}

void updateGameOver()
{
    if ( button_x.click() )
    {
        esp_restart();
    }

    drawGameOver();
}

void initCanvas()
{
    canvasWidth = canvasSize * 2.0f;
    canvasHeight = canvasWidth;

    printf( "canvasWidth: %u\n", canvasWidth );

    canvasCenterX = canvasWidth * 0.5f;
    canvasCenterY = canvasHeight * 0.5f;

    printFreeHeap();

    canvas.setColorDepth( canvasBitsPerPixel );
    canvas.createSprite( canvasWidth, canvasHeight );
    
    printFreeHeap();

    canvas.setTextDatum( MC_DATUM );
    canvas.setTextColor( whiteColor, backgroundColor );
    canvas.loadFont( Montserrat32 );
}

void setup()
{
    Serial.begin( 115200 );
    Serial.println();

    printf( "game started\n" );

    // display

    display.init();
    display.setRotation( 1 );

    display.fillScreen( backgroundColor );

    // NVS

    NVS.begin();

    // random seed

    srand( esp_random() );

    // canvas

    initCanvas();

    // game

    createPlayerSprite( playerSprite );
    createTailSprite( tailSprite );
    createSquareSprite( squareSprite );
    createWallSprite( wallSprite );

    updatePlayerPosition();
    setSquareRandomPosition();
    updateWallPosition();

    initWallAngles();

    spawnWall( 0 );
    spawnWall( 3 );

    updateTailSpacing();

    load();
}

void loop()
{
    float deltaTime = gameClock.calculateDeltaTime();

    switch ( gameState )
    {
        case GameState::MainMenu:
            updateMainMenu( deltaTime );
            break;
        case GameState::Gameplay:
            updateGameplay( deltaTime );
            break;
        case GameState::GameOver:
            updateGameOver();
            break;
    }
}

