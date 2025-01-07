#pragma once

//#include "CameraParams.h"    

#define PXIEL_FORMAT_FLOAT32 0x81200001                  // ch:float32像素格式 | en:Float32 PixelFormat

#define TEMP_REGION_COUNT 22                             // ch:全部区域个数 | en:Number of all region
#define TEMP_MAX_POINT_NUM 10                            // ch:点区域最大个数 | en：Maximum number of Point region
#define TEMP_MAX_POLY_NUM 10                             // ch:多边形区域最大个数 | en:Maximum number of Polygon region
#define TEMP_MAX_LINE_NUM 1                              // ch:线区域最大个数 | en:Maximum number of Line region
#define TEMP_MAX_CIRCLE_NUM 1                            // ch:圆区域最大个数 | en:Maximum number of Circle region

#define TEMP_MAX_POLY_LINE_COUNT 20                      // ch:多边形区域边的最大个数 | en:Maximum number of Polygon region edges
#define TEMP_MAX_MULTI_ALARM_COUNT 4                     // ch:多区域报警可配置的组数 | en:Configurable number of groups for Multi Temperature Region Alarm Rule
#define TEMP_MAX_DISPLAY_WINDOW_COUNT 5                  // ch:显示设置窗口可配置的组数 | en:Display the number of groups that can be configured in the settings window

#define TEMP_CHUNK_ID_TEST 0x00510002                    // ch:测温区域信息 | en:Temp Measurement region information
#define TEMP_CHUNK_ID_ALARM 0x00510003                   // ch:测温警告信息 | en:Temp Measurement alarm information
#define TEMP_CHUNK_ID_RAW_DATA 0x00510004                // ch:全屏灰度数据 | en:Full ScreenRaw data
#define TEMP_CHUNK_ID_FULL_SCREEN_DATA 0x00510005        // ch:全屏温度数据 | en:Full Screen Temperature data
#define TEMP_CHUNK_ID_MIN_MAX_TEMP 0x00510006            // ch:最低温最高温 | en:Min Temp & Max Temp
#define TEMP_CHUNK_ID_OSD_INFO 0x00510007                // ch:OSD相关参数 | en:OSD Related parameters

#define TEMP_ROI_TYPE_POINT 0
#define TEMP_ROI_TYPE_LINE 1
#define TEMP_ROI_TYPE_POLYGON 2
#define TEMP_ROI_TYPE_CIRCLE 3

#define TEMP_ALARM_LEVER_PRE 0        // ch:报警等级-预警 | en；Alarm level-Early warning
#define TEMP_ALARM_LEVER_WARN 1       // ch:报警等级-警告 | en:Alarm level-Warning
#define TEMP_ALARM_LEVER_NORMAL 2     // ch:报警等级-正常 | en:Alarm level-Normal
#define TEMP_ALARM_LEVER_RECOVER 3    // ch:报警等级-解除等级 | en:Alarm level-Recover
#define TEMP_ALARM_TYPE_MAX 0         // ch:报警类型-最大值 | en:Alarm type-Maximum Temperature Difference
#define TEMP_ALARM_TYPE_MIN 1         // ch:报警类型-最小值 | en:Alarm type-Minimum Temperature Difference
#define TEMP_ALARM_TYPE_AVG 2         // ch:报警类型-平均值 | en:Alarm type-Average Temperature Difference
#define TEMP_ALARM_TYPE_DIFFER 3      // ch:报警类型-差异值 | en:Alarm type-Variation Temperature Difference

#define OSD_BY_NONE    0           // ch:OSD不显示 | en:OSD do not display
#define OSD_BY_CAMERA  1           // ch:相机显示OSD | en:Over Screen by Camera
#define OSD_BY_CLIENT  2           // ch:软件层显示OSD | en:Over Screen by Client



// ch:设置的ROI类型 | en:ROI Action
enum ROIActionType
{
	ROI_ACTION_TYPE_POINT = 0,       // ch:点 | en:point
	ROI_ACTION_TYPE_POLYGON,         // ch:多边形 | en:polygon
	ROI_ACTION_TYPE_LINE,            // ch:线 | en:line
	ROI_ACTION_TYPE_CIRCLE,          // ch:圆 | en:circle
	ROI_ACTION_TYPE_UNDEFINE,
};

// ch:报警源 | en:Alarm Source
enum AlarmSourceType
{
	ALARM_SOURCE_TYPE_MAX = 0,    // ch:最大温度 | en:Maximum Temperature
	ALARM_SOURCE_TYPE_MIN,        // ch:最小温度 | en:Minimum Temperature
	ALARM_SOURCE_TYPE_AVG,        // ch:平均温度 | en:Average Temperature
	ALARM_SOURCE_TYPE_VAR,        // ch:温度差异 | en:Temperature Difference
};

// ch:报警条件 | en：Alarm Condition
enum AlarmConditionType
{
	ALARM_CONDITION_TYPE_MORE = 0,    // ch:大于 | en:More
	ALARM_CONDITION_TYPE_LESS,        // ch:小于 | en:Less
	ALARM_CONDITION_TYPE_NONE,        // ch:无 | en:None
};


typedef struct
{
    unsigned char pointNum;     // ch:点测温个数，最大10个 | en:Number of Point Temperature region,Max:Ten
    unsigned char boxNum;       // ch:框测温个数，最大10个 | en:Number of polygon Temperature region,Max:Ten
    unsigned char lineNum;      // ch:线测温个数，最多1条 | en:Number of Line Temperature region,Max:One
	unsigned char circleNum;    // ch:圆测温个数，最多1个 | en:Number of Circle Temperature region,Max:One
    unsigned char total;        // ch:上者之和 | en:All of the above
	unsigned char res[3];
} IFR_TM_REGION_NUM;

// ch:点 | en：Point
typedef struct
{
    int x;
    int y;
} IFR_POINT;


// ch:多边形区域 | en:Polygon region
typedef struct
{
	unsigned int pointNum;              // <ch:多边形实际顶点数 | en:Point of Polygon region
	unsigned int circleRadius;          // <ch:测温区域圆半径 | en:CircleRadius of Temperature Measurement region
	IFR_POINT circlepoint;              // <ch:测温区域的圆心点 | en:Circlepoint of Temperature Measurement region
	IFR_POINT pointList[TEMP_REGION_COUNT];            // <ch:顶点坐标 | en:PointList
} IFR_POLYGON;


// ch:单个区域测温结果 | en:Temp Measurement Result of Temperature Region
typedef struct
{
    unsigned char       enable;             /// <ch:是否启用：0-否,1-是 | en:Enable: 0-disable,1-Enable
    unsigned char       regionId;           /// <ch:区域ID | en:RegionId
    unsigned char       reserved[34];       /// <ch:保留字段 | en:Reserved Fields
    unsigned int        regiontype;         /// <ch:区域类型 0：点 1：线 2：多边形  3:圆 | en:Region Type 0:point 1:Line 2:Polygon
    char                name[32];           /// <ch:区域名称 | en:Region Name
    unsigned int        emissionRate;       /// <ch:发射率: [1，100] | en:Emissivity
    int                 minTmp;             /// <ch:最低温度: [-400, 10000]，单位0.1℃ | en:Minimum Temperature: [-400, 10000]，Unit:0.1℃
    int                 maxTmp;             /// <ch:最高温度: [-400, 10000]，单位0.1℃ | en:Maximum Temperature: [-400, 10000]，Unit:0.1℃
    int                 avrTmp;             /// <ch:平均温度: [-400, 10000]，单位0.1℃ | en:Average Temperature: [-400, 10000]，Unit:0.1℃
    int                 diffTmp;            /// <ch:温差： [0, 10400]，单位0.1℃ | en:Temperature variation:[0, 10400],Unit:0.1℃
    IFR_POINT           points[2];          /// <ch:保存测试结果中的最高温和最低温坐标，数组下标: 0-最高温，1-最低温 | en:Save Maximum Temperature  and Minimum Temperature coordinates in the test results,Array subscript:0-Maximum Temperature,1-Minimum Temperature
    IFR_POLYGON         polygon;            /// <ch:多边形区域 | en:Polygon region
} IFR_OUTCOME_INFO;


// ch:温度结果列表 | en:Temperature Results List
typedef struct
{
    IFR_TM_REGION_NUM   regionNum;              /// <ch:有效测温区域数量 | en:Number of effective temperature measurement region
    unsigned char       reserved[8];            /// <ch:保留 | en:Reserve
	IFR_OUTCOME_INFO    ifrOutcome[TEMP_REGION_COUNT];         /// <ch:测温结果 | en:Temperature measurement
} IFR_OUTCOME_LIST;

// ch:报警上传信息结构体 | en::Uploading information structure of Alarm
typedef struct
{
    unsigned char regionId;                     /// <ch:框序号 | en:RegionId
    unsigned char alarmkey;                     /// <ch:当前区域报警是否开启开关，上层默认配置开启 | en:Whether the current region alarm is on or off, the upper layer is on by default
    unsigned char alarmRule;                    /// <ch:规则:0-大于 1-小于 | en:Rule:0-greater than 1-Less than
	unsigned char reserved0;                    /// <ch:预留 | en:Reserve
    unsigned int regiontype;                    /// <ch:区域类型 0：点 1：线 2：多边形 3:圆 | en:Regiontype 0：point 1：Line 2：Polygon 3:Circle
    unsigned int alarmType;                     /// <ch:报警类型:0-最高温报警,1-最低温度，2-平均温度,3-温差 | en:AlarmType:0-Maximum Temperature alarm,1-Minimum Temperature,2-Average Temperature,3-Temperature variation
    unsigned int alarmLevel;                    /// <ch:报警级别：0-预警，1-报警，2-正常，3-解除预警 | en:Alarm Level:0-Early waining,2-Normal,3-Cancel early warning
    int measureTmpData;                         /// <ch:测量值，单位0.1℃ | en:Temperature Measurements,Unit0.1℃
    int ruleTmpData;                            /// <ch:规则设定值，单位0.1℃ | en:Rule Settings，Unit0.1℃
    IFR_POLYGON polygon;                        /// <ch:区域位置 | en:Region Location
    IFR_POINT points[2];                        /// <ch:保存测试结果中的最高温和最低温坐标，数组下标: 0-最高温，1-最低温 | en:Save the highest temperature and lowest temperature coordinates in the test results,Array subscript:0-Maximum Temperature,1-Minimum Temperature
	unsigned char reserved1[16];                /// <ch:预留 | en:Reserve
} IFR_ALARM_INFO;


// ch:区域温差报警上传信息结构体 | en:Uploading information structure of  Multi Temperature Region Alarm
typedef struct
{
    unsigned char regionSet[2];                 /// <ch:温差报警区域ID号，每对2个区域 | en:Temperature Variation alarm region ID,One ID for two regions
	unsigned char alarmkey;                     /// <ch:当前区域报警是否开启开关 | en:Whether the current region alarm is on
	unsigned char reserved0;                    /// <ch:预留 | en:Reserve
    unsigned int alarmType;                     /// <ch:报警类型:0-最高温报警,1-最低温度，2-平均温度,3-温差 | en:AlarmType:0-Maximum Temperature alarm,1-Minimum Temperature,2-Average Temperature,3-Temperature variation
    unsigned char alarmRule;                    /// <ch:规则:0-大于 1-小于 | en:Rule:0-greater than 1-Less than
    unsigned char reserved1[3];
    unsigned int alarmLevel;                    /// <ch:报警级别：0-预警，1-报警，2-正常，3-解除预警 | en:Alarm Level:0-Early waining,2-Normal,3-Cancel early warning
	int measureTmpData[2];                      /// <ch:温度报警区域测量值，单位0.1℃，与regionSet对应 | en:Temperature alarm region measurement,Unit 0.1℃，Corresponds to regionSet
    int ruleTmpData;                            /// <ch:规则设定值，单位0.1℃ | en:Rule Settings，Unit0.1℃
    unsigned char reserved2[4];                 /// <ch:预留 | en:Reserve
} IFR_DIFF_ALARM_INFO;


typedef struct
{
	IFR_ALARM_INFO alarmOutcome[TEMP_REGION_COUNT]; /// ch:单区域报警结果 | en:Temperature Region Alarm result
    IFR_DIFF_ALARM_INFO alarmDiffOutcome[4];         /// ch:区域间温差报警结果 | en: Multi Temperature Region Alarm result
}IFR_ALARM_UPLOAD_INFO;

// ch:全屏温度信息 | en:Full Screen Temperature information
typedef struct
{
    int nMaxTemp;
    int nMinTemp;
    unsigned char reserved[4];
} IFR_FULL_SCREEN_MAX_MIN_INFO;

// ch:测温区域显示规则 | en:Temperature region Display Rule
typedef struct {
	unsigned int    regionDispIndex;                    /**< ch:区域显示规则索引号 | en:Region Display rule Index number */ 
	unsigned int    regionDispEnable;                   /**< ch:测温区域显示使能 | en: Temp Measurement region Display Enable */ 
	unsigned int    regionMaxTempDispEnable;            /**< ch:区域最大温度显示使能 | en:Region Maximum Temperature Display Enable */ 
	unsigned int    regionMinTempDispEnable;            /**< ch:区域最小温度显示使能 | en:Region Minimum  Temperature Display Enable*/ 
	unsigned int    regionAvgTempDispEnable;            /**< ch:区域平均温度显示使能 | en:Region Average Temperature Display Enable*/ 
	unsigned int    regionAlarmDispEnable;              /**< ch:区域告警状态显示使能 | en:Region Alarm Temperature Display Enable*/
} IRF_REGION_DISP_INFO;

// ch:区域测温图像叠加特征信息结构体 | en:Information Structure of superimposed feature  of region temperature measurement image
typedef struct {
	unsigned int           legendDisplayEnable;    /**< ch:图例（温度条）是否使能 1 使能 0 不使能 | en:Legend: 1:Enable 0:Disable*/
	unsigned int           osdProcessor;           /**< ch:红外相机OSD叠加控制器选择 0 无叠加 1 相机叠加 2 客户端叠加 | en:Over Screen Display Processor: 0:None 1:Camera 2:Client*/
	IRF_REGION_DISP_INFO   regionDispRules[TEMP_REGION_COUNT];
} IRF_OSD_INFO;





